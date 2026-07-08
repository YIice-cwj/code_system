#include "error_system/core/error_context.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "error_system/config/feature_flags.h"
#include "error_system/config/error_config.h"
#include "error_system/core/serializer/error_context_serializer.h"
#include "error_system/utils/bad_alloc_handler.h"
#include "error_system/utils/log.h"
#include "error_system/utils/source_location.h"
#include "error_system/utils/stack_trace_utils.h"
#include "serializer/error_context_serializer_internal.h"

/**
 * @file error_context.cc
 * @brief 错误上下文实现（24 字节 Move-Only 重构版）
 * @details 物理布局：8B error_code + 8B runtime_block* + 8B cause* = 24 字节。
 *          动态字段全部收拢到 runtime_block_t 堆块，按需分配。
 *          runtime_block_t 通过线程本地对象池复用，消除高频错误路径堆分配开销。
 *          因果链用 unique_ptr 串联，零引用计数。
 *          Move-Only：禁用拷贝，仅移动。
 * @author yiice
 * @version 4.3.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 确保 block_ 已分配
     * @details 首次写入动态字段时从线程本地对象池获取 runtime_block_t。
     *          池分配失败（含堆分配失败）时记录日志并保持空 pool_ptr_t。
     */
    void error_context_t::ensure_block_() noexcept {
        if (!block_) {
            block_ = utils::pool_ptr_t<runtime_block_t>(
                utils::object_pool_t<runtime_block_t>::acquire());
        }
    }

    /**
     * @brief 查找 payload 值指针
     */
    const std::string* error_context_t::find_payload_(const std::string& key) const noexcept {
        if (!block_) {
            return nullptr;
        }
        for (size_t i = 0; i < block_->payload_count && i < PAYLOAD_SSO_CAPACITY; ++i) {
            if (block_->payload_small[i].first == key) {
                return &block_->payload_small[i].second;
            }
        }
        if (block_->payload_overflow) {
            auto it = block_->payload_overflow->find(key);
            if (it != block_->payload_overflow->end()) {
                return &it->second;
            }
        }
        return nullptr;
    }

    /**
     * @brief 检测指定对象是否在当前 cause 链中
     * @details 沿 cause 链向下遍历，深度上限 MAX_CAUSE_DEPTH。
     *          用于 wrap() 防止循环引用。
     */
    bool error_context_t::has_cause_in_chain_(const error_context_t* target) const noexcept {
        if (target == nullptr) {
            return false;
        }
        const error_context_t* current = cause_.get();
        size_t depth = 0;
        while (current != nullptr && depth < MAX_CAUSE_DEPTH) {
            if (current == target) {
                return true;
            }
            current = current->cause_.get();
            ++depth;
        }
        return false;
    }

    /**
     * @brief 修复源位置指针（反序列化后重新指向 loc_file_storage_）
     */
    void error_context_t::repair_source_location_pointers_() noexcept {
        if (block_ && !block_->loc_file_storage.empty()) {
            block_->file_name = block_->loc_file_storage.c_str();
            block_->source_location = utils::source_location_t(
                block_->loc_file_storage.c_str(), block_->loc_func_storage.c_str(),
                block_->source_location.line());
        }
    }

    /**
     * @brief 深拷贝当前错误上下文（含 cause 链递归克隆）
     * @details block_ 通过 runtime_block_t::deep_copy 从池获取内存复制；
     *          cause_ 递归调用 clone() 重建独立 cause 链。
     *          分配失败时返回部分拷贝。
     */
    error_context_t error_context_t::clone() const noexcept {
        error_context_t copy{};
        copy.code_ = code_;
        if (block_) {
            try {
                copy.block_ = runtime_block_t::deep_copy(*block_);
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("error_context", "clone");
            }
        }
        if (cause_) {
            try {
                copy.cause_ = std::make_unique<error_context_t>(cause_->clone());
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("error_context", "clone_cause");
            }
        }
        return copy;
    }

    /**
     * @brief 包装底层错误为当前错误的直接原因（移动语义）
     * @details 深拷贝当前对象的 runtime_block_（保留源状态），将 underlying 移动到 cause_。
     *          自环检测：若 underlying 是 this 本身或已在 this 的 cause 链中，跳过 cause 设置。
     *          分配失败时 cause 保持空并记录日志，不抛异常。
     */
    error_context_t error_context_t::wrap(error_context_t&& underlying) const noexcept {
        error_context_t new_context{};
        new_context.code_ = code_;
        if (block_) {
            try {
                new_context.block_ = runtime_block_t::deep_copy(*block_);
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("error_context", "wrap");
            }
        }
        if (&underlying == this || underlying.has_cause_in_chain_(this)) {
            LOG_ERROR("[error_context] wrap: cycle detected, skipping cause");
            return new_context;
        }
        try {
            new_context.cause_ = std::make_unique<error_context_t>(std::move(underlying));
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context", "wrap_cause");
        }
        return new_context;
    }

    /**
     * @brief 获取堆栈帧（已符号化字符串）
     * @details 优先返回 resolved_frames（测试手动设置的）；否则对 raw_frames 执行
     *          延迟符号化（带 thread_local 缓存，二次调用 O(1)）。
     * @return 堆栈帧 shared_ptr，未分配 block_ 或无帧时返回 nullptr
     */
    std::shared_ptr<const std::vector<std::string>> error_context_t::get_stack_frames() const noexcept {
        if (!block_) { return nullptr; }
        if (block_->resolved_frames) { return block_->resolved_frames; }
        if (!block_->raw_frames) { return nullptr; }
        try {
            return std::make_shared<const std::vector<std::string>>(
                utils::stack_trace_utils_t::resolve(*block_->raw_frames));
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context", "get_stack_frames");
            return nullptr;
        }
    }

    /**
     * @brief 设置已符号化的堆栈帧
     * @details 用于测试场景手动设置堆栈帧。生产代码中堆栈帧由构造时自动捕获
     *          （存为 raw_frames，输出时延迟符号化）。
     * @param frames 堆栈帧字符串列表
     * @return 自身引用（支持链式调用）
     */
    error_context_t& error_context_t::with_stack_frames(std::vector<std::string> frames) noexcept {
        ensure_block_();
        if (block_) {
            try {
                block_->resolved_frames = std::make_shared<const std::vector<std::string>>(std::move(frames));
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("error_context", "with_stack_frames");
            }
        }
        return *this;
    }

    /**
     * @brief 获取所有 payload 的副本
     * @return 键值对向量，分配失败时返回空向量
     */
    std::vector<std::pair<std::string, std::string>> error_context_t::get_payload() const noexcept {
        std::vector<std::pair<std::string, std::string>> result;
        try {
            result.reserve(payload_size());
            for_each_payload([&](const std::string& key, const std::string& value) {
                result.emplace_back(key, value);
            });
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context", "get_payload");
        }
        return result;
    }

    /**
     * @brief 按 key 查找 payload 值
     * @param key payload 键名
     * @return 对应的值，未找到或分配失败时返回 nullopt
     */
    std::optional<std::string> error_context_t::get_payload_value(const std::string& key) const noexcept {
        try {
            const std::string* found = find_payload_(key);
            if (found != nullptr) {
                return *found;
            }
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context", "get_payload_value");
        }
        return std::nullopt;
    }

    /**
     * @brief 序列化为可读文本
     * @details 委托 error_context_serializer_t::to_string 实现
     * @return 文本格式的错误上下文
     */
    std::string error_context_t::to_string() const noexcept {
        return error_context_serializer_t::to_string(*this);
    }

    /**
     * @brief 序列化为 JSON 字符串
     * @details 委托 error_context_serializer_t::to_json 实现
     * @return JSON 格式的错误上下文
     */
    std::string error_context_t::to_json() const noexcept {
        return error_context_serializer_t::to_json(*this);
    }

    /**
     * @brief 序列化为紧凑二进制
     * @details 委托 error_context_serializer_t::to_binary 实现
     * @return 二进制格式的错误上下文
     */
    std::string error_context_t::to_binary() const noexcept {
        return error_context_serializer_t::to_binary(*this);
    }

    /**
     * @brief 比较两个错误上下文（code/message/payload）
     * @param other 待比较对象
     * @return bool true=相等（不含 cause 链与 stack_frames 比较）
     */
    bool error_context_t::operator==(const error_context_t& other) const noexcept {
        if (code_.get_code() != other.code_.get_code()) {
            return false;
        }
        const std::string_view this_msg = block_ ? std::string_view(block_->message) : std::string_view{};
        const std::string_view other_msg = other.block_ ? std::string_view(other.block_->message) : std::string_view{};
        if (this_msg != other_msg) {
            return false;
        }
        const size_t this_size = payload_size();
        if (this_size != other.payload_size()) {
            return false;
        }
        bool equal = true;
        for_each_payload([&](const std::string& key, const std::string& value) {
            if (!equal) {
                return;
            }
            const std::string* other_value = other.find_payload_(key);
            if (other_value == nullptr || *other_value != value) {
                equal = false;
            }
        });
        return equal;
    }

    /**
     * @brief 严格相等比较（含 cause 链与 stack_frames 深比较）
     * @details operator== 仅比较 code/message/payload；本方法额外比较 cause 链与堆栈，
     *          适用于完整状态比对（如测试断言、缓存键）。
     *          超过 MAX_CAUSE_DEPTH(32) 时停止递归返回 false，防止循环引用导致栈溢出。
     */
    bool error_context_t::equals_strict(const error_context_t& other, size_t depth) const noexcept {
        if (depth >= MAX_CAUSE_DEPTH) {
            return false;
        }
        if (!(*this == other)) {
            return false;
        }
        if constexpr (error_system::config::feature_flags_t::STACKTRACE_ENABLED) {
            const auto this_frames = get_stack_frames();
            const auto other_frames = other.get_stack_frames();
            const bool this_has = static_cast<bool>(this_frames);
            const bool other_has = static_cast<bool>(other_frames);
            if (this_has != other_has) {
                return false;
            }
            if (this_has && *this_frames != *other_frames) {
                return false;
            }
        }
        if (cause_ && other.cause_) {
            return cause_->equals_strict(*other.cause_, depth + 1);
        }
        return !cause_ && !other.cause_;
    }

    /**
     * @brief 构造最小化错误上下文
     * @details 跳过 validation/stacktrace/notification，不分配 block_。
     *          仅供 result_t<T, true>（Lean 模式）的读取路径使用。
     * @param code 错误码
     * @param location 源位置
     * @return 最小化错误上下文
     */
    error_context_t error_context_t::make_minimal(error_code_t code,
                                                  utils::source_location_t location) noexcept {
        error_context_t ctx{};
        ctx.code_ = code;
        ctx.ensure_block_();
        ctx.block_->source_location = location;
        return ctx;
    }

    /**
     * @brief 聚合多个错误上下文为一个
     * @details 主错误取第一个，其余以 joined_error_N 为键作为 payload 附加。
     *          std::to_string 与 string 拼接可能抛 bad_alloc，用 try-catch 包裹，
     *          失败时停止追加后续错误（已追加的保留）。
     * @param errors 错误上下文列表（将被移动）
     * @return 聚合后的错误上下文
     */
    error_context_t join_errors(std::vector<error_context_t>&& errors) noexcept {
        if (errors.empty()) {
            return error_context_t{};
        }
        if (errors.size() == 1) {
            return std::move(errors[0]);
        }
        error_context_t primary = std::move(errors[0]);
        try {
            for (size_t i = 1; i < errors.size(); ++i) {
                std::string key = "joined_error_" + std::to_string(i);
                const std::string_view msg = errors[i].block_ ? std::string_view(errors[i].block_->message) : std::string_view{};
                primary.with(std::move(key), msg);
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[join_errors] std::bad_alloc");
        }
        return primary;
    }

    /**
     * @brief 校验错误码合法性并修复未注册错误码
     * @details 查询注册表，若错误码未注册则标记为 fatal 并附加 "[UNREGISTERED CODE]" 前缀，
     *          同时记录原始错误码到 payload。仅在 feature_flags_t::is_validation_enabled()
     *          时执行，校验失败时回退到 system_domain 的 fallback 错误码。
     */
    void error_context_t::fill_validation_fields_() noexcept {
        if (!config::feature_flags_t::is_validation_enabled()) {
            return;
        }
        auto info = error_registry_t::instance().get_info_cached(code_);
        if (info) {
            return;
        }
        try {
            with("illegal_raw_code", std::to_string(code_.get_code()));
            if (block_) {
                std::string prefixed_message = "[UNREGISTERED CODE] ";
                prefixed_message += std::string_view(block_->message);
                block_->message = std::move(prefixed_message);
            }
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context", "fill_validation_fields_");
        }
        constexpr uint16_t FALLBACK_ERROR_NUMBER = 0xFFFF;
        code_ = error_code_t(error_level_t::fatal, domain::system_domain_t::none,
                             subsystem_id_t{0}, module_id_t{0},
                             error_number_t{FALLBACK_ERROR_NUMBER});
        ensure_block_();
        if (block_) {
            block_->metadata = error_registry_t::instance().get_info_cached(code_);
        }
    }

    /**
     * @brief 抓取当前线程调用栈
     * @details 仅在编译期 STACKTRACE_ENABLED 开启且运行期 stacktrace_enabled 时执行。
     *          错误级别需达到 stacktrace_config_t 配置的阈值（支持 per_code 覆盖）。
     *          堆栈帧以 raw_frames 形式存储，输出时延迟符号化。
     */
    void error_context_t::fill_stacktrace_() noexcept {
        if constexpr (!config::feature_flags_t::STACKTRACE_ENABLED) {
            return;
        }
        if (!config::feature_flags_t::is_stacktrace_enabled()) {
            return;
        }
        auto stacktrace_level = config::stacktrace_config_t::get_stacktrace_level();
        const auto per_code_level = config::stacktrace_config_t::get_per_code_stacktrace_level(
            code_.get_identity_code());
        if (per_code_level.has_value()) {
            stacktrace_level = per_code_level.value();
        }
        if (code_.get_level() < stacktrace_level) {
            return;
        }
        try {
            ensure_block_();
            if (block_) {
                block_->raw_frames = std::make_shared<const std::vector<void*>>(
                    utils::stack_trace_utils_t::capture(1));
            }
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context", "fill_stacktrace_");
        }
    }

    /**
     * @brief 判断是否携带可用的源位置信息
     * @details 编译期未启用 LOCATION_ENABLED 时返回 false；运行时开关关闭时返回 false；
     *          动态块未分配或 file_name 为空时返回 false。
     * @return bool 是否有源位置
     */
    bool error_context_t::is_location_available() const noexcept {
        if constexpr (!config::feature_flags_t::LOCATION_ENABLED) {
            return false;
        } else {
            if (!config::feature_flags_t::is_source_location_enabled()) {
                return false;
            }
            const auto* blk = block();
            return blk != nullptr && blk->file_name != nullptr;
        }
    }

    /**
     * @brief 填充源位置信息
     * @details 仅在编译期 LOCATION_ENABLED 开启且运行期 location_enabled 时执行。
     *          short_filename_enabled 为 true 时提取短文件名，否则保留完整路径。
     * @param short_filename_enabled 是否使用短文件名
     */
    void error_context_t::fill_source_location_(bool short_filename_enabled) noexcept {
        if constexpr (!config::feature_flags_t::LOCATION_ENABLED) {
            return;
        }
        if (!config::feature_flags_t::is_source_location_enabled()) {
            return;
        }
        ensure_block_();
        if (!block_) {
            return;
        }
        const char* src = block_->source_location.file_name();
        block_->file_name = short_filename_enabled ? utils::extract_short_filename(src) : src;
    }

}  // namespace error_system::core
