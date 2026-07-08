#pragma once
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/core/runtime_block.h"
#include "error_system/utils/log.h"
#include "error_system/utils/object_pool.h"
#include "error_system/utils/source_location.h"
#include "error_system/utils/stack_trace_utils.h"
#include "error_system/utils/string_format.h"

/**
 * @file error_context.h
 * @brief 错误上下文数据类定义（24 字节 Move-Only）
 * @details 物理布局：8B error_code + 8B runtime_block* + 8B cause* = 24 字节。
 *          动态字段（message/payload/source_location/stack_frames/metadata）全部收拢到
 *          runtime_block_t 堆块中，按需分配（nullptr 表示无动态数据，零开销）。
 *          runtime_block_t 通过线程本地对象池复用，消除高频错误路径的堆分配开销。
 *          因果链采用 std::unique_ptr<error_context_t> 独占所有权，零引用计数。
 *          类为 Move-Only：禁用拷贝构造/赋值，仅保留移动。
 * @author yiice
 * @version 4.3.2
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 携带源位置的错误码包装
     * @details 从 error_code_t 隐式构造时，自动捕获调用者位置。
     * @param err_code 错误码
     * @param src_loc 源位置（默认捕获调用者位置）
     */
    struct located_code_t {
        error_code_t code;
        utils::source_location_t location;

        located_code_t(error_code_t err_code, utils::source_location_t src_loc = utils::source_location_t::current()) noexcept
            : code(err_code), location(src_loc) {}
    };

    /**
     * @brief 错误上下文数据类（24 字节 Move-Only）
     * @details 封装完整的错误上下文信息。物理布局严格 24 字节：
     *          - 8B error_code（身份凭证）
     *          - 8B runtime_block_t*（动态字段堆块指针，nullptr=无动态数据）
     *          - 8B error_context_t* cause（因果链独占指针）
     *
     *          动静分离设计：
     *          - 静态数据（error_code）内联，零堆访问
     *          - 动态数据（message/payload/stack/source_location）收拢到 runtime_block_t
     *          - 因果链用 unique_ptr 串联，零引用计数开销
     *
     *          Move-Only 语义：禁用拷贝，仅支持移动。如需共享，调用方自行用 shared_ptr 包装。
     *
     * @example 基础用法
     * error_context_t context(ERR_DB_TIMEOUT, "连接超时: {}ms", 3000);
     * context.with("host", "192.168.1.1")
     *        .with("retry_count", "3");
     * std::cout << error_context_serializer_t::to_string(context) << "\n";
     */
    class error_context_t {
    public:
        static constexpr size_t PAYLOAD_SSO_CAPACITY = 4;

    private:
        friend class error_context_serializer_t;
        friend error_context_t join_errors(std::vector<error_context_t>&& errors) noexcept;

        error_code_t code_{};
        utils::pool_ptr_t<runtime_block_t> block_{};
        std::unique_ptr<error_context_t> cause_{};

        /**
         * @brief 确保 block_ 已分配
         * @details 首次写入动态字段时按需分配 runtime_block_t。
         *          分配失败时记录日志并保持 nullptr。
         */
        void ensure_block_() noexcept;

        /**
         * @brief 查找 payload 值指针
         * @return payload 值指针，未找到时返回 nullptr
         */
        const std::string* find_payload_(const std::string& key) const noexcept;

        /**
         * @brief 插入或更新 payload 字段
         * @details 前 4 项写入 payload_small_（SSO），超出后溢出到 payload_overflow_。
         * @tparam Key 键类型
         * @tparam V 值类型
         * @param key 键
         * @param value 值
         * @return 自身引用
         */
        template <typename Key, typename V>
        error_context_t& insert_or_update_payload_(Key&& key, V&& value) noexcept;

        /**
         * @brief 检测指定对象是否在当前 cause 链中
         * @details 用于 wrap() 自环检测，避免循环引用。
         * @param target 待检测对象指针
         * @return 在 cause 链中返回 true，否则 false
         */
        bool has_cause_in_chain_(const error_context_t* target) const noexcept;

        /**
         * @brief 修复源位置指针
         * @details 反序列化后重新指向 loc_file_storage_，确保生命周期安全。
         */
        void repair_source_location_pointers_() noexcept;

        /**
         * @brief 校验错误码合法性并修复未注册错误码
         * @details 查询注册表，若错误码未注册则标记为 fatal 并附加 "[UNREGISTERED CODE]" 前缀。
         *          仅在 feature_flags_t::is_validation_enabled() 时执行。
         */
        void fill_validation_fields_() noexcept;

        /**
         * @brief 抓取当前线程调用栈
         * @details 仅在 feature_flags_t::is_stacktrace_enabled() 且错误级别达标时执行。
         *          堆栈帧以 raw_frames 形式存储，输出时延迟符号化。
         */
        void fill_stacktrace_() noexcept;

        /**
         * @brief 根据 short_filename_enabled 设置 file_name 字段
         * @details 仅在 feature_flags_t::is_source_location_enabled() 时执行。
         * @param short_filename_enabled 是否使用短文件名
         */
        void fill_source_location_(bool short_filename_enabled) noexcept;

    public:
        error_context_t() noexcept = default;
        error_context_t(const error_context_t&) = delete;
        error_context_t& operator=(const error_context_t&) = delete;
        error_context_t(error_context_t&&) noexcept = default;
        error_context_t& operator=(error_context_t&&) noexcept = default;
        ~error_context_t() noexcept = default;

        /**
         * @brief 构造函数（接受 located_code_t，自动捕获调用位置和堆栈）
         * @details 在构造时完成以下操作：
         *          1. 格式化消息字符串
         *          2. 通过 located_code_t 捕获源位置
         *          3. 根据全局配置校验错误码、抓取堆栈、通知插件
         *          若错误码 sign=0（成功），则跳过步骤 3 且不分配 block_。
         * @tparam Args 格式化参数类型包
         * @param located_code 携带源位置的错误码
         * @param message_format 错误信息格式化字符串
         * @param args 格式化参数列表
         */
        template <typename... Args>
        error_context_t(located_code_t located_code, std::string message_format, Args&&... args) noexcept;

        /**
         * @brief 深拷贝当前错误上下文（含 cause 链递归克隆）
         * @details 用于需要从 const 引用创建独立副本的场景（如通知系统异步入队、
         *          延迟缓冲）。Move-Only 语义下不可拷贝，需显式调用 clone()。
         *          分配失败时返回部分拷贝（已分配字段保留，未分配字段为空）。
         * @return 深拷贝的新错误上下文
         */
        [[nodiscard]] error_context_t clone() const noexcept;

        /**
         * @brief 包装底层错误为当前错误的直接原因
         * @details 当前对象被深拷贝到新对象，underlying 被移动到新对象的 cause 链。
         *          若检测到循环引用（&underlying == this 或 underlying 的 cause 链中
         *          包含 this）则跳过 cause 设置。
         *          采用右值引用签名以确保自环检测时地址比较有效
         *          （按值签名会触发移动构造，地址失效）。
         * @param underlying 底层错误上下文（将被移动）
         * @return 包含因果链的新错误上下文
         */
        [[nodiscard]] error_context_t wrap(error_context_t&& underlying) const noexcept;

        /**
         * @brief 获取堆栈帧（已符号化字符串）
         * @details 优先返回 resolved_frames（测试手动设置的）；否则对 raw_frames 执行
         *          延迟符号化（带 thread_local 缓存，二次调用 O(1)）。
         * @return 堆栈帧 shared_ptr，未分配 block_ 或无帧时返回 nullptr
         */
        [[nodiscard]] std::shared_ptr<const std::vector<std::string>> get_stack_frames() const noexcept;

        /**
         * @brief 设置已符号化的堆栈帧
         * @details 用于测试场景手动设置堆栈帧。生产代码中堆栈帧由构造时自动捕获
         *          （存为 raw_frames，输出时延迟符号化）。
         * @param frames 堆栈帧字符串列表
         * @return 自身引用（支持链式调用）
         */
        error_context_t& with_stack_frames(std::vector<std::string> frames) noexcept;

        /**
         * @brief 添加多类型 payload（int/double/bool 等）
         * @details 字符串类型（std::string, const char*, std::string_view）优先匹配非模板重载。
         * @tparam T 值类型
         * @param key 字段名
         * @param value 字段值
         * @return 自身引用（支持链式调用）
         */
        template <typename T>
        error_context_t& with(const std::string& key, T value) noexcept;
        template <typename T>
        error_context_t& with(const char* key, T value) noexcept;
        template <typename T>
        error_context_t& with(std::string_view key, T value) noexcept;

        /**
         * @brief 遍历所有 payload 项
         * @details 对 SSO 和溢出存储中的所有项依次调用 visitor。
         * @tparam Visitor 访问者可调用对象类型
         * @param visitor 接受 (const std::string& key, const std::string& value) 的回调
         */
        template <typename Visitor>
        void for_each_payload(Visitor&& visitor) const noexcept;

        /**
         * @brief 获取所有 payload 的副本
         * @return payload 键值对列表
         */
        [[nodiscard]] std::vector<std::pair<std::string, std::string>> get_payload() const noexcept;

        /**
         * @brief 按 key 查找 payload 值
         * @param key 字段名
         * @return 找到则返回值，未找到返回 std::nullopt
         */
        [[nodiscard]] std::optional<std::string> get_payload_value(const std::string& key) const noexcept;

        /**
         * @brief 完整比较（code + message + payload）
         * @param other 另一个错误上下文
         * @return 相等返回 true，否则 false
         */
        [[nodiscard]] bool operator==(const error_context_t& other) const noexcept;

        /**
         * @brief 严格比较（含 cause 链深比较）
         * @details operator== 仅比较 code/message/payload；本方法额外比较 cause 链与堆栈。
         * @param other 另一个错误上下文
         * @param depth 递归深度（默认 0）
         * @return 相等返回 true，否则 false
         */
        [[nodiscard]] bool equals_strict(const error_context_t& other, size_t depth = 0) const noexcept;

        /**
         * @brief 只读访问动态上下文块
         * @return 块指针，未分配时为 nullptr
         */
        [[nodiscard]] const runtime_block_t* block() const noexcept { return block_.get(); }

        /**
         * @brief 判断是否携带可用的源位置信息
         * @details 编译期未启用 LOCATION_ENABLED 时返回 false；运行时开关关闭时返回 false；
         *          动态块未分配或 file_name 为空时返回 false。供序列化器复用，消除重复判断。
         * @return bool 是否有源位置
         */
        [[nodiscard]] bool is_location_available() const noexcept;

        /**
         * @brief 只读访问因果链下游节点
         * @return cause 指针，无因果链时为 nullptr
         */
        [[nodiscard]] const error_context_t* cause() const noexcept { return cause_.get(); }

        /**
         * @brief 获取错误码
         * @return 错误码 const 引用
         */
        [[nodiscard]] const error_code_t& get_code() const noexcept { return code_; }

        /**
         * @brief 获取错误消息
         * @return 消息字符串视图，未分配 block_ 时返回空视图
         */
        [[nodiscard]] std::string_view get_message() const noexcept {
            return block_ ? std::string_view(block_->message) : std::string_view{};
        }

        /**
         * @brief 获取源文件名
         * @return 文件名指针，未分配 block_ 时返回 nullptr
         */
        [[nodiscard]] const char* get_file_name() const noexcept {
            return block_ ? block_->file_name : nullptr;
        }

        /**
         * @brief 获取源位置信息
         * @return 源位置引用，未分配 block_ 时返回空哨兵
         */
        [[nodiscard]] const utils::source_location_t& get_source_location() const noexcept {
            static const utils::source_location_t empty{};
            return block_ ? block_->source_location : empty;
        }

        /**
         * @brief 获取错误码元数据（统一反查入口）
         * @details 优先复用 block_->metadata（构造时已缓存），
         *          未缓存时回退到 registry 查询。消除序列化器中的重复反查。
         * @return 元数据 optional，未注册时为 nullopt
         */
        [[nodiscard]] std::optional<error_metadata_t> get_metadata() const noexcept {
            if (block_ && block_->metadata) {
                return block_->metadata;
            }
            return error_registry_t::instance().get_info_cached(code_);
        }

        /**
         * @brief 是否为成功状态（sign=0）
         * @return 成功返回 true，否则 false
         */
        [[nodiscard]] bool is_success() const noexcept { return code_.is_success_code(); }

        /**
         * @brief 是否为失败状态（sign!=0）
         * @return 失败返回 true，否则 false
         */
        [[nodiscard]] bool is_error() const noexcept { return code_.is_error_code(); }

        /**
         * @brief 是否为 fatal 级别
         * @return fatal 级别返回 true，否则 false
         */
        [[nodiscard]] bool is_fatal() const noexcept { return code_.get_level() == error_level_t::fatal; }

        /**
         * @brief 是否可重试
         * @return 可重试返回 true，否则 false
         */
        [[nodiscard]] bool is_retryable() const noexcept { return code_.is_retryable(); }

        /**
         * @brief 是否为瞬态
         * @return 瞬态返回 true，否则 false
         */
        [[nodiscard]] bool is_transient() const noexcept { return code_.is_transient(); }

        /**
         * @brief 添加字符串键值 payload
         * @param key 字段名
         * @param value 字段值
         * @return 自身引用（支持链式调用）
         */
        error_context_t& with(const std::string& key, const std::string& value) noexcept {
            return insert_or_update_payload_(key, value);
        }

        /**
         * @brief 添加字符串键值 payload（C 字符串重载）
         * @param key 字段名
         * @param value 字段值
         * @return 自身引用（支持链式调用）
         */
        error_context_t& with(const char* key, const char* value) noexcept {
            return with(std::string_view(key != nullptr ? key : ""), std::string_view(value != nullptr ? value : ""));
        }

        /**
         * @brief 添加字符串键值 payload（string_view 重载）
         * @param key 字段名
         * @param value 字段值
         * @return 自身引用（支持链式调用）
         */
        error_context_t& with(std::string_view key, std::string_view value) noexcept {
            try {
                return insert_or_update_payload_(std::string(key), std::string(value));
            } catch (const std::bad_alloc&) {
                LOG_ERROR("[error_context] with(string_view): std::bad_alloc");
                return *this;
            }
        }

        /**
         * @brief 添加字符串键值 payload（右值重载）
         * @param key 字段名
         * @param value 字段值
         * @return 自身引用（支持链式调用）
         */
        error_context_t& with(std::string&& key, std::string&& value) noexcept {
            return insert_or_update_payload_(std::move(key), std::move(value));
        }

        /**
         * @brief 批量添加 payload
         * @param items 键值对列表
         * @return 自身引用（支持链式调用）
         */
        error_context_t& with_batch(std::initializer_list<std::pair<const std::string, std::string>> items) noexcept {
            for (const auto& [key, value] : items) {
                with(key, value);
            }
            return *this;
        }

        /**
         * @brief 获取 payload 项数
         * @return payload 项数
         */
        [[nodiscard]] size_t payload_size() const noexcept {
            if (!block_) {
                return 0;
            }
            if (block_->payload_overflow) {
                return block_->payload_overflow->size();
            }
            return block_->payload_count;
        }

        /**
         * @brief payload 是否为空
         * @return 为空返回 true，否则 false
         */
        [[nodiscard]] bool is_payload_empty() const noexcept { return payload_size() == 0; }

        /**
         * @brief 获取 C 字符串形式的消息
         * @return 消息 C 字符串，未分配 block_ 时返回空字符串
         */
        [[nodiscard]] const char* what() const noexcept {
            return block_ ? block_->message.c_str() : "";
        }

        /**
         * @brief 序列化为可读文本
         * @return 序列化后的文本
         */
        [[nodiscard]] std::string to_string() const noexcept;

        /**
         * @brief 序列化为 JSON 字符串
         * @return 序列化后的 JSON 字符串
         */
        [[nodiscard]] std::string to_json() const noexcept;

        /**
         * @brief 序列化为紧凑二进制
         * @return 序列化后的二进制字符串
         */
        [[nodiscard]] std::string to_binary() const noexcept;

        /**
         * @brief 不相等比较
         * @param other 另一个错误上下文
         * @return 不相等返回 true，否则 false
         */
        [[nodiscard]] bool operator!=(const error_context_t& other) const noexcept { return !(*this == other); }

        /**
         * @brief 仅按错误码比较
         * @param other 另一个错误上下文
         * @return 错误码相同返回 true，否则 false
         */
        [[nodiscard]] bool equals_by_code(const error_context_t& other) const noexcept {
            return code_.get_code() == other.code_.get_code();
        }

        /**
         * @brief 从标准异常创建错误上下文
         * @param code 错误码
         * @param exception 标准异常
         * @param location 源位置
         * @return 构造好的错误上下文
         */
        [[nodiscard]] static error_context_t from_exception(error_code_t code, const std::exception& exception,
                                                              utils::source_location_t location = utils::source_location_t::current()) noexcept {
            return error_context_t(located_code_t{code, location}, exception.what());
        }

        /**
         * @brief 构造最小化错误上下文
         * @details 跳过 validation/stacktrace/notification，通过 ensure_block_() 分配 block_
         *          仅用于存储 source_location。仅供 result_t<T, true>（Lean 模式）的读取路径使用。
         * @param code 错误码
         * @param location 源位置
         * @return 最小化错误上下文
         */
        [[nodiscard]] static error_context_t make_minimal(error_code_t code,
                                                          utils::source_location_t location = utils::source_location_t::current()) noexcept;
    };

    /**
     * @brief 聚合多个错误为单个错误上下文
     * @details 将多个错误上下文合并为一个，payload 中保留各错误的详细信息。
     * @param errors 错误上下文列表（将被移动）
     * @return 聚合后的错误上下文
     */
    [[nodiscard]] error_context_t join_errors(std::vector<error_context_t>&& errors) noexcept;

}  // namespace error_system::core

#include "error_system/core/details/error_context.inl"
