#pragma once
#include "error_system/config/error_config.h"
#include "error_system/core/error_context.h"

namespace error_system::core {

    /**
     * @brief 插入或更新 payload 字段（核心逻辑）
     * @details 统一处理 SSO 查找→溢出查找→SSO 追加→溢出迁移四个分支。
     *          所有动态字段写入 block_，首次写入时按需分配。
     * @tparam Key 键类型（支持 const string&/string&&/string_view）
     * @tparam V 值类型
     * @param key 键
     * @param value 值
     * @return error_context_t& 自身引用
     */
    template <typename Key, typename V>
    inline error_context_t& error_context_t::insert_or_update_payload_(Key&& key, V&& value) noexcept {
        ensure_block_();
        if (!block_) {
            return *this;
        }
        try {
            const size_t sso_end = std::min<size_t>(block_->payload_count, PAYLOAD_SSO_CAPACITY);
            for (size_t i = 0; i < sso_end; ++i) {
                if (block_->payload_small[i].first == key) {
                    block_->payload_small[i].second = std::forward<V>(value);
                    return *this;
                }
            }

            if (block_->payload_overflow) {
                block_->payload_overflow->insert_or_assign(std::forward<Key>(key), std::forward<V>(value));
                return *this;
            }

            if (block_->payload_count < PAYLOAD_SSO_CAPACITY) {
                block_->payload_small[block_->payload_count].first = std::forward<Key>(key);
                block_->payload_small[block_->payload_count].second = std::forward<V>(value);
                ++block_->payload_count;
                return *this;
            }

            auto overflow = std::make_unique<std::unordered_map<std::string, std::string>>();
            for (size_t i = 0; i < PAYLOAD_SSO_CAPACITY; ++i) {
                overflow->emplace(std::move(block_->payload_small[i].first), std::move(block_->payload_small[i].second));
            }
            overflow->insert_or_assign(std::forward<Key>(key), std::forward<V>(value));
            block_->payload_overflow = std::move(overflow);
            for (size_t i = 0; i < PAYLOAD_SSO_CAPACITY; ++i) {
                block_->payload_small[i] = {};
            }
            block_->payload_count = 0;
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[error_context] insert_or_update_payload_: std::bad_alloc\n");
        }
        return *this;
    }

    /**
     * @brief 构造函数（接受 located_code_t，自动捕获调用位置和堆栈）
     * @details 在构造时自动完成以下操作：
     *          1. 分配 runtime_block_t 并格式化消息
     *          2. 通过 located_code_t 捕获源位置（调用者真实位置）
     *          3. 从注册表查询共享只读 metadata（零深拷贝）
     *          4. 若为错误码：校验错误码、抓取堆栈、通知插件
     *          若错误码 sign=0（成功），则跳过步骤 4
     * @tparam Args 格式化参数类型包
     * @param located_code 携带源位置的错误码（支持从 error_code_t 隐式构造）
     * @param message_format 错误信息格式化字符串（支持 {} 占位符）
     * @param args 格式化参数列表
     */
    template <typename... Args>
    inline error_context_t::error_context_t(located_code_t located_code, std::string message_format, Args&&... args) noexcept
        : code_(located_code.code) {
        ensure_block_();
        if (block_) {
            try {
                block_->message = utils::string_format_t::format(std::move(message_format), std::forward<Args>(args)...);
            } catch (const std::bad_alloc&) {
                std::fprintf(stderr, "[error_context] constructor: message format failed (bad_alloc)\n");
            }
            try {
                block_->metadata = error_registry_t::instance().get_info_cached(located_code.code);
            } catch (const std::bad_alloc&) {
                std::fprintf(stderr, "[error_context] constructor: metadata lookup failed\n");
            }
            if constexpr (error_system::config::feature_flags_t::LOCATION_ENABLED) {
                block_->source_location = located_code.location;
            }
        }
        if (is_success()) {
            return;
        }
        error_context_initializer_t::initialize(*this);
    }

    /**
     * @brief 添加多类型负载字段（模板版本）
     * @details 根据 T 的类型自动选择转换方式：
     *          - bool → "true"/"false"
     *          - 算术类型（int, double 等）→ std::to_string
     *          - 其他类型 → static_cast<std::string>
     *          字符串类型（std::string, const char*, std::string_view）优先匹配非模板重载
     * @tparam T 值类型
     * @param key 字段名
     * @param value 字段值
     * @return error_context_t& 当前对象引用，支持链式调用
     */
    template <typename T>
    inline error_context_t& error_context_t::with(const std::string& key, T value) noexcept {
        try {
            if constexpr (std::is_same_v<T, bool>) {
                return with(key, std::string(value ? "true" : "false"));
            } else if constexpr (std::is_arithmetic_v<T>) {
                return with(key, std::to_string(value));
            } else {
                return with(key, std::string(value));
            }
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[error_context] with(const string&, T): std::bad_alloc\n");
            return *this;
        }
    }

    /**
     * @brief 添加自定义负载（const char* key）
     * @details 使用 const char* 作为 key 添加任意类型值到错误上下文
     * @tparam T 值类型
     * @param key 键名
     * @param value 值
     * @return error_context_t& 自身引用
     */
    template <typename T>
    inline error_context_t& error_context_t::with(const char* key, T value) noexcept {
        if (key == nullptr) {
            std::fprintf(stderr, "[error_context] with(const char*, T): key is nullptr\n");
            return *this;
        }
        try {
            return with(std::string(key), std::move(value));
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[error_context] with(const char*, T): std::bad_alloc\n");
            return *this;
        }
    }

    /**
     * @brief 添加自定义负载（string_view key）
     * @details 使用 std::string_view 作为 key 添加任意类型值到错误上下文
     * @tparam T 值类型
     * @param key 键名
     * @param value 值
     * @return error_context_t& 自身引用
     */
    template <typename T>
    inline error_context_t& error_context_t::with(std::string_view key, T value) noexcept {
        try {
            return with(std::string(key), std::move(value));
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[error_context] with(string_view, T): std::bad_alloc\n");
            return *this;
        }
    }

    /**
     * @brief 遍历所有 payload 项
     * @details 对 SSO 和溢出存储中的所有项依次调用 visitor。
     *          block_ 为 nullptr 时直接返回（零开销）。
     * @tparam Visitor 访问者可调用对象类型
     * @param visitor 接受 (const std::string& key, const std::string& value) 的回调
     */
    template <typename Visitor>
    inline void error_context_t::for_each_payload(Visitor&& visitor) const noexcept {
        if (!block_) {
            return;
        }
        for (size_t i = 0; i < block_->payload_count && i < PAYLOAD_SSO_CAPACITY; ++i) {
            visitor(block_->payload_small[i].first, block_->payload_small[i].second);
        }
        if (block_->payload_overflow) {
            for (const auto& [key, value] : *block_->payload_overflow) {
                visitor(key, value);
            }
        }
    }

}  // namespace error_system::core
