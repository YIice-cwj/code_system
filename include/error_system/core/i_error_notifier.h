#pragma once
#include "error_system/core/error_code.h"

/**
 * @file i_error_notifier.h
 * @brief 错误通知器抽象接口
 * @details 解耦 core 层对 plugin 层的反向依赖。core 层通过此接口通知错误事件，
 *          plugin 层提供具体实现（如 plugin_registry_t）。遵循依赖倒置原则。
 *          接口收敛为两个 notify 重载：完整上下文（Full 模式）与纯错误码（Lean 模式）。
 *          通知模式（sync/async_queue/sync_deferred）由实现内部根据 feature_flags 分发，
 *          调用方无需关心。
 *          全局通知器指针通过 set_current/get_current 静态方法管理。
 * @author yiice
 * @version 4.3.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    class error_context_t;

    /**
     * @brief 错误通知器接口
     * @details 由 plugin 层实现，core 层通过 i_error_notifier_t::get_current()
     *          获取当前实例并调用 notify 触发通知，避免直接依赖 plugin 层。
     * @warning 所有方法声明为 noexcept。实现类必须内部 try-catch 所有可能抛异常的代码
     *          （如内存分配、插件回调、字符串构造），未捕获的异常将导致 std::terminate。
     * @see plugin_registry_t
     */
    class i_error_notifier_t {
    private:
        static i_error_notifier_t* current_;

    public:
        virtual ~i_error_notifier_t() noexcept = default;

        /**
         * @brief 通知完整错误上下文（Full 模式入口）
         * @details 实现内部根据 feature_flags_t::get_notify_mode() 分发到
         *          sync/async_queue/sync_deferred 路径，调用方无需关心通知模式。
         *          由 result_t<T, false>::make_error 在构造 error_context_t 后显式调用。
         * @param context 错误上下文
         */
        virtual void notify(const error_context_t& context) noexcept = 0;

        /**
         * @brief 通知纯错误码（Lean 模式入口）
         * @details 始终走无锁队列异步通道，不构造 error_context_t。
         *          后台线程出队后反查 error_registry_t 构造轻量 error_context_t 并驱动插件。
         *          由 result_t<T, true>::make_error 调用。
         * @param code 错误码
         */
        virtual void notify(error_code_t code) noexcept = 0;

        /**
         * @brief 设置当前错误通知器
         * @details 注入 i_error_notifier_t 实现，解耦 core 层对 plugin 层的直接依赖。
         *          通常由 plugin_registry_t::instance() 自注册，或应用启动时显式注入。
         *          传 nullptr 可清除已有通知器。
         * @note 非线程安全，预期在初始化阶段（通知发生前）调用一次。
         * @param notifier 通知器指针
         */
        static void set_current(i_error_notifier_t* notifier) noexcept;

        /**
         * @brief 获取当前错误通知器
         * @return 通知器指针，未设置时返回 nullptr
         */
        [[nodiscard]] static i_error_notifier_t* get_current() noexcept;

        /**
         * @brief 若已设置通知器则通知完整上下文，否则空操作
         * @details 封装 get_current + nullptr 检查 + notify 三步，消除调用方重复代码。
         *          result_t::make_error 与测试代码统一通过本方法触发通知。
         * @param context 错误上下文
         */
        static void try_notify(const error_context_t& context) noexcept;

        /**
         * @brief 若已设置通知器则通知纯错误码，否则空操作
         * @details 封装 get_current + nullptr 检查 + notify 三步，消除调用方重复代码。
         * @param code 错误码
         */
        static void try_notify(error_code_t code) noexcept;
    };

}  // namespace error_system::core
