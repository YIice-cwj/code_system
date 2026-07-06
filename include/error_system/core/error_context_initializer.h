#pragma once

/**
 * @file error_context_initializer.h
 * @brief 错误上下文初始化器
 * @details 从 error_context_t 拆分而来，单一职责：在 error_context_t 构造时
 *          根据全局配置完成错误码校验、堆栈捕获、源位置记录和插件通知。
 *          本头文件仅前向声明 error_context_t 与 i_error_notifier_t（不包含
 *          error_context.h / i_error_notifier.h），error_context.h 在类定义前
 *          包含本文件以避免循环依赖。
 *          插件通知通过 i_error_notifier_t 抽象接口完成，core 层不再直接依赖
 *          plugin 层（依赖倒置原则）。
 * @author yiice
 * @version 3.0.0
 * @date 2026-06-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    class error_context_t;
    class i_error_notifier_t;

    /**
     * @brief 错误上下文初始化器
     * @details 封装 error_context_t 构造后的运行时特性初始化逻辑。
     *          类仅包含静态成员，禁止实例化。通过 error_context_t 的 friend 声明
     *          访问其私有成员 code_ 与 metadata_。
     *          通知路径通过 set_error_notifier() 注入的 i_error_notifier_t 实例完成，
     *          解耦对 plugin_registry_t 的直接依赖。
     */
    class error_context_initializer_t {
    private:
        static i_error_notifier_t* notifier_;

        /**
         * @brief 执行错误码合法性校验
         * @details 查询注册表，若错误码未注册则标记为 fatal 并附加 "[UNREGISTERED CODE]" 前缀。
         */
        static void fill_validation_fields_(error_context_t& context) noexcept;

        /**
         * @brief 抓取当前线程调用栈
         */
        static void fill_stacktrace_(error_context_t& context) noexcept;

        /**
         * @brief 根据 short_filename_enabled 设置 context.file_name
         */
        static void fill_source_location_(error_context_t& context, bool short_filename_enabled) noexcept;

    public:
        error_context_initializer_t() = delete;
        ~error_context_initializer_t() = delete;
        error_context_initializer_t(const error_context_initializer_t&) = delete;
        error_context_initializer_t& operator=(const error_context_initializer_t&) = delete;
        error_context_initializer_t(error_context_initializer_t&&) = delete;
        error_context_initializer_t& operator=(error_context_initializer_t&&) = delete;

        /**
         * @brief 执行运行时特性初始化
         * @details 根据全局配置依次完成：错误码校验 → 堆栈捕获 → 源位置记录 → 插件通知。
         *          成功码上下文（sign=0）由调用方在调用前自行跳过。
         *          插件通知通过 i_error_notifier_t 接口完成，若未设置通知器则跳过通知。
         * @param context 待初始化的错误上下文
         */
        static void initialize(error_context_t& context) noexcept;

        /**
         * @brief 设置错误通知器
         * @details 注入 i_error_notifier_t 实现，解耦 core 层对 plugin 层的直接依赖。
         *          通常由 plugin_registry_t::instance() 自注册，或应用启动时显式注入。
         *          传 nullptr 可清除已有通知器。
         * @note 非线程安全，预期在初始化阶段（通知发生前）调用一次。
         * @param notifier 通知器指针
         */
        static void set_error_notifier(i_error_notifier_t* notifier) noexcept {
            notifier_ = notifier;
        }

        /**
         * @brief 获取当前错误通知器
         * @return 通知器指针，未设置时返回 nullptr
         */
        [[nodiscard]] static i_error_notifier_t* get_error_notifier() noexcept {
            return notifier_;
        }
    };

}  // namespace error_system::core
