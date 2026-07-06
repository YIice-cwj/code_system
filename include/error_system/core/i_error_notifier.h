#pragma once

/**
 * @file i_error_notifier.h
 * @brief 错误通知器抽象接口
 * @details 解耦 core 层对 plugin 层的反向依赖。core 层通过此接口通知错误事件，
 *          plugin 层提供具体实现（如 plugin_registry_t）。遵循依赖倒置原则。
 *          接口覆盖三种通知路径：同步通知、异步入队、延迟累积。
 *          本头文件仅前向声明 error_context_t，避免与 error_context.h 形成循环依赖。
 * @author yiice
 * @version 3.0.0
 * @date 2026-06-29
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    class error_context_t;

    /**
     * @brief 错误通知器接口
     * @details 由 plugin 层实现，core 层通过 error_context_initializer_t::set_error_notifier()
     *          注入具体实现，避免直接依赖 plugin 层。
     * @warning 所有方法声明为 noexcept。实现类必须内部 try-catch 所有可能抛异常的代码
     *          （如内存分配、插件回调、字符串构造），未捕获的异常将导致 std::terminate。
     * @see error_context_initializer_t::set_error_notifier
     * @see plugin_registry_t
     */
    class i_error_notifier_t {
    public:
        virtual ~i_error_notifier_t() noexcept = default;

        /**
         * @brief 同步通知错误事件
         * @details 立即向所有已注册插件分发错误上下文，调用方阻塞直至所有插件处理完成。
         *          sync 通知模式下由 error_context_t 构造时调用。
         */
        virtual void notify_error(const error_context_t& context) noexcept = 0;

        /**
         * @brief 异步入队错误通知
         * @details 将错误上下文副本推入后台队列，由工作线程异步处理，调用方不阻塞。
         *          async_queue 通知模式下由 error_context_t 构造时调用。
         */
        virtual void enqueue_notification(const error_context_t& context) noexcept = 0;

        /**
         * @brief 累积延迟通知到线程本地缓冲
         * @details 通知累积到当前线程的本地缓冲中，直至显式 flush 时统一批量通知。
         *          适用于请求处理等批处理场景。sync_deferred 通知模式下由
         *          error_context_t 构造时调用。
         */
        virtual void enqueue_deferred_notification(const error_context_t& context) noexcept = 0;
    };

}  // namespace error_system::core
