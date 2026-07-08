#pragma once
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "error_system/core/error_level.h"
#include "error_system/core/registry/duplicate_policy.h"

/**
 * @file feature_flags.h
 * @brief 特性开关与运行时配置类
 * @details 单一职责：管理错误系统的编译期特性开关与运行时配置项。
 *          涵盖：编译期特性开关（堆栈/验证/源位置）、运行时布尔标志、
 *          通知模式与通知通道容量、注册表重复处理策略。
 *          编译期常量通过 if constexpr 消除运行时开销，由编译器死代码消除未启用分支。
 * @author yiice
 * @version 4.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::config {

    /**
     * @brief 特性开关配置类
     * @details 封装错误系统的所有特性开关，所有特性开关编译期决议，分支由编译器死代码消除。
     *          类仅包含静态成员，禁止实例化。
     */
    class feature_flags_t {
    public:
        /**
         * @brief 插件通知模式
         * @details sync：同步通知（默认），由 result_t::make_error 经 i_error_notifier_t::try_notify()
         *          立即分发到所有插件；
         *          async_queue：异步模式，通知推入内部队列，由工作线程消费；
         *          sync_deferred：同步延迟模式，通知累积到线程本地缓冲，
         *                         由调用方显式调用 flush_deferred_notifications() 触发批量通知
         */
        enum class notify_mode_t : uint8_t {
            sync = 0,
            async_queue = 1,
            sync_deferred = 2,
        };

        static constexpr bool STACKTRACE_ENABLED =
#ifdef ERROR_SYSTEM_ENABLE_STACKTRACE
            true;
#else
            false;
#endif
        static constexpr bool VALIDATION_ENABLED =
#ifdef ERROR_SYSTEM_ENABLE_VALIDATION
            true;
#else
            false;
#endif
        static constexpr bool LOCATION_ENABLED =
#ifdef ERROR_SYSTEM_ENABLE_LOCATION
            true;
#else
            false;
#endif

    private:
        /**
         * @brief 是否启用堆栈追踪标志位存储
         * @details 使用 std::atomic<bool> 保证无锁并发读写
         * @return std::atomic<bool>& 是否启用堆栈追踪标志位引用
         */
        static std::atomic<bool>& get_flag_stacktrace_() noexcept;

        /**
         * @brief 是否启用错误码验证标志位存储
         * @details 使用 std::atomic<bool> 保证无锁并发读写
         * @return std::atomic<bool>& 是否启用错误码验证标志位引用
         */
        static std::atomic<bool>& get_flag_validation_() noexcept;

        /**
         * @brief 是否启用错误源位置(文件/行号)标志位存储
         * @details 使用 std::atomic<bool> 保证无锁并发读写
         * @return std::atomic<bool>& 是否启用错误源位置(文件/行号)标志位引用
         */
        static std::atomic<bool>& get_flag_source_location_() noexcept;

        /**
         * @brief 是否启用缩短源文件名标志位存储
         * @details 使用 std::atomic<bool> 保证无锁并发读写
         * @return std::atomic<bool>& 是否启用缩短源文件名标志位引用
         */
        static std::atomic<bool>& get_flag_short_filename_() noexcept;

        /**
         * @brief 通知模式存储
         * @details 使用 std::atomic<notify_mode_t> 保证无锁并发读写
         * @return std::atomic<notify_mode_t>& 通知模式存储引用
         */
        static std::atomic<notify_mode_t>& get_notify_mode_() noexcept;

        /**
         * @brief 异步通知队列最大容量存储
         * @details async_queue 模式下队列上限，0 表示无限制
         * @return std::atomic<size_t>& 队列容量存储引用
         */
        static std::atomic<size_t>& get_async_queue_max_size_() noexcept;

        /**
         * @brief 延迟通知缓冲最大容量存储
         * @details sync_deferred 模式下线程本地缓冲上限，0 表示无限制
         * @return std::atomic<size_t>& 缓冲容量存储引用
         */
        static std::atomic<size_t>& get_deferred_buffer_max_size_() noexcept;

        /**
         * @brief 注册表重复处理策略存储
         * @details 使用 std::atomic<uint8_t> 存储 duplicate_policy_t 枚举值
         * @return std::atomic<uint8_t>& 策略存储引用
         */
        static std::atomic<uint8_t>& get_duplicate_policy_() noexcept;

    public:
        feature_flags_t() = delete;
        ~feature_flags_t() = delete;
        feature_flags_t(const feature_flags_t&) = delete;
        feature_flags_t& operator=(const feature_flags_t&) = delete;
        feature_flags_t(feature_flags_t&&) = delete;
        feature_flags_t& operator=(feature_flags_t&&) = delete;

        /**
         * @brief 全局开启/关闭堆栈追踪功能
         * @details 通过 std::atomic<bool> 无锁设置。若编译期未启用堆栈追踪，此调用无实际操作。
         * @param enable 是否开启
         */
        static void set_enable_stacktrace(bool enable) noexcept;

        /**
         * @brief 检查全局堆栈追踪功能是否开启
         * @details 若编译期未启用堆栈追踪，始终返回 false，
         *          允许编译器进行死代码消除 (Dead Code Elimination)。
         * @return bool 是否开启
         */
        [[nodiscard]] static bool is_stacktrace_enabled() noexcept;

        /**
         * @brief 全局开启/关闭错误码验证功能
         * @details 若编译期未启用验证，此调用无实际操作。
         * @param enable 是否开启
         */
        static void set_enable_validation(bool enable) noexcept;

        /**
         * @brief 检查错误码验证功能是否开启
         * @details 若编译期未启用验证，始终返回 false，
         *          允许编译器进行死代码消除 (Dead Code Elimination)。
         * @return bool 是否开启
         */
        [[nodiscard]] static bool is_validation_enabled() noexcept;

        /**
         * @brief 全局开启/关闭错误位置功能
         * @details 若编译期未启用位置追踪，此调用无实际操作。
         * @param enable 是否开启
         */
        static void set_enable_source_location(bool enable) noexcept;

        /**
         * @brief 检查错误位置功能是否开启
         * @details 若编译期未启用位置追踪，始终返回 false，
         *          允许编译器进行死代码消除 (Dead Code Elimination)。
         * @return bool 是否开启
         */
        [[nodiscard]] static bool is_source_location_enabled() noexcept;

        /**
         * @brief 设置是否开启文件名缩写
         * @details 若编译期未启用位置追踪，此调用无实际操作。
         * @param enable 是否开启
         */
        static void set_enable_short_filename(bool enable) noexcept;

        /**
         * @brief 检查文件名缩写功能是否开启
         * @details 若编译期未启用位置追踪，始终返回 false，
         *          允许编译器进行死代码消除 (Dead Code Elimination)。
         * @return bool 是否开启
         */
        [[nodiscard]] static bool is_short_filename_enabled() noexcept;

        /**
         * @brief 设置插件通知模式
         * @param mode 通知模式
         */
        static void set_notify_mode(notify_mode_t mode) noexcept;

        /**
         * @brief 获取插件通知模式
         * @return notify_mode_t 当前通知模式
         */
        [[nodiscard]] static notify_mode_t get_notify_mode() noexcept;

        /**
         * @brief 设置异步通知队列最大容量
         * @details async_queue 模式下队列达到上限时新通知被丢弃，0 表示无限制
         * @param max_size 队列最大容量
         */
        static void set_async_queue_max_size(size_t max_size) noexcept;

        /**
         * @brief 获取异步通知队列最大容量
         * @return size_t 队列最大容量，0 表示无限制
         */
        [[nodiscard]] static size_t get_async_queue_max_size() noexcept;

        /**
         * @brief 设置延迟通知缓冲最大容量
         * @details sync_deferred 模式下线程本地缓冲上限，0 表示无限制
         * @param max_size 缓冲最大容量
         */
        static void set_deferred_buffer_max_size(size_t max_size) noexcept;

        /**
         * @brief 获取延迟通知缓冲最大容量
         * @return size_t 缓冲最大容量，0 表示无限制
         */
        [[nodiscard]] static size_t get_deferred_buffer_max_size() noexcept;

        /**
         * @brief 设置注册表重复处理策略
         * @param policy 重复处理策略
         */
        static void set_duplicate_policy(core::duplicate_policy_t policy) noexcept;

        /**
         * @brief 获取注册表重复处理策略
         * @return duplicate_policy_t 当前策略
         */
        [[nodiscard]] static core::duplicate_policy_t get_duplicate_policy() noexcept;
    };

    inline std::atomic<bool>& feature_flags_t::get_flag_stacktrace_() noexcept {
        static std::atomic<bool> enabled{true};
        return enabled;
    }

    inline std::atomic<bool>& feature_flags_t::get_flag_validation_() noexcept {
        static std::atomic<bool> enabled{true};
        return enabled;
    }

    inline std::atomic<bool>& feature_flags_t::get_flag_source_location_() noexcept {
        static std::atomic<bool> enabled{true};
        return enabled;
    }

    inline std::atomic<bool>& feature_flags_t::get_flag_short_filename_() noexcept {
        static std::atomic<bool> enabled{true};
        return enabled;
    }

    inline std::atomic<feature_flags_t::notify_mode_t>& feature_flags_t::get_notify_mode_() noexcept {
        static std::atomic<feature_flags_t::notify_mode_t> mode{feature_flags_t::notify_mode_t::sync};
        return mode;
    }

    inline std::atomic<size_t>& feature_flags_t::get_async_queue_max_size_() noexcept {
        static std::atomic<size_t> max_size{0};
        return max_size;
    }

    inline std::atomic<size_t>& feature_flags_t::get_deferred_buffer_max_size_() noexcept {
        static std::atomic<size_t> max_size{1024};
        return max_size;
    }

    inline std::atomic<uint8_t>& feature_flags_t::get_duplicate_policy_() noexcept {
        static std::atomic<uint8_t> policy{static_cast<uint8_t>(core::duplicate_policy_t::skip)};
        return policy;
    }

    inline void feature_flags_t::set_enable_stacktrace(bool enable) noexcept {
        if constexpr (STACKTRACE_ENABLED) {
            get_flag_stacktrace_().store(enable);
        }
    }

    inline bool feature_flags_t::is_stacktrace_enabled() noexcept {
        if constexpr (STACKTRACE_ENABLED) {
            return get_flag_stacktrace_().load();
        } else {
            return false;
        }
    }

    inline void feature_flags_t::set_enable_validation(bool enable) noexcept {
        if constexpr (VALIDATION_ENABLED) {
            get_flag_validation_().store(enable);
        }
    }

    inline bool feature_flags_t::is_validation_enabled() noexcept {
        if constexpr (VALIDATION_ENABLED) {
            return get_flag_validation_().load();
        } else {
            return false;
        }
    }

    inline void feature_flags_t::set_enable_source_location(bool enable) noexcept {
        if constexpr (LOCATION_ENABLED) {
            get_flag_source_location_().store(enable);
        }
    }

    inline bool feature_flags_t::is_source_location_enabled() noexcept {
        if constexpr (LOCATION_ENABLED) {
            return get_flag_source_location_().load();
        } else {
            return false;
        }
    }

    inline void feature_flags_t::set_enable_short_filename(bool enable) noexcept {
        if constexpr (LOCATION_ENABLED) {
            get_flag_short_filename_().store(enable);
        }
    }

    inline bool feature_flags_t::is_short_filename_enabled() noexcept {
        if constexpr (LOCATION_ENABLED) {
            return get_flag_short_filename_().load();
        } else {
            return false;
        }
    }

    inline void feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t mode) noexcept {
        get_notify_mode_().store(mode);
    }

    inline feature_flags_t::notify_mode_t feature_flags_t::get_notify_mode() noexcept {
        return get_notify_mode_().load();
    }

    inline void feature_flags_t::set_async_queue_max_size(size_t max_size) noexcept {
        get_async_queue_max_size_().store(max_size);
    }

    inline size_t feature_flags_t::get_async_queue_max_size() noexcept {
        return get_async_queue_max_size_().load();
    }

    inline void feature_flags_t::set_deferred_buffer_max_size(size_t max_size) noexcept {
        get_deferred_buffer_max_size_().store(max_size);
    }

    inline size_t feature_flags_t::get_deferred_buffer_max_size() noexcept {
        return get_deferred_buffer_max_size_().load();
    }

    inline void feature_flags_t::set_duplicate_policy(core::duplicate_policy_t policy) noexcept {
        get_duplicate_policy_().store(static_cast<uint8_t>(policy));
    }

    inline core::duplicate_policy_t feature_flags_t::get_duplicate_policy() noexcept {
        return static_cast<core::duplicate_policy_t>(get_duplicate_policy_().load());
    }

}  // namespace error_system::config
