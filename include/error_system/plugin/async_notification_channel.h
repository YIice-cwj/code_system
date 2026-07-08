#pragma once
#include <cstddef>
#include <cstdio>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <utility>

#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/utils/async_queue.h"
#include "error_system/utils/lock_free_queue.h"

/**
 * @file async_notification_channel.h
 * @brief 异步通知通道
 * @details 提供两条异步通知路径：
 *          1. error_context 通道：基于 async_queue_t，将完整错误上下文异步入队。
 *          2. error_code 通道：基于 mpsc_queue_t 无锁队列，将纯错误码异步入队，
 *             专为 result_lean 设计，调用方数纳秒完成，后台线程反查注册表后驱动插件。
 *          通过构造时注入回调，将出队数据转发给实际通知者（plugin_registry_t），
 *          打破与具体注册表实现的循环依赖，遵循单一职责原则（SRP）。
 * @author yiice
 * @version 4.4.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::plugin {

    /**
     * @brief 异步通知通道
     * @details 管理两条独立的后台通知路径，各自拥有工作线程。
     *          error_context 路径用于完整模式的异步通知；
     *          error_code 路径用于 Lean 模式的纯错误码通知。
     */
    class async_notification_channel_t {
    public:
        /** @brief 错误上下文的共享指针类型 */
        using context_ptr_t = std::shared_ptr<core::error_context_t>;
        /** @brief 错误上下文通知回调函数类型 */
        using notify_callback_t = std::function<void(const core::error_context_t&)>;
        /** @brief 纯错误码通知回调函数类型 */
        using code_callback_t = std::function<void(core::error_code_t)>;

    private:
        /**
         * @brief 出队上下文处理器
         * @details 持有通知回调，将出队的 shared_ptr<error_context_t> 解引用后转发给回调。
         *          通过回调注入避免与 plugin_registry_t 产生循环依赖。
         */
        struct context_processor_t {
            notify_callback_t callback;

            void operator()(context_ptr_t& context) const noexcept {
                if (callback && context) {
                    callback(*context);
                }
            }
        };

        utils::async_queue_t<context_ptr_t, context_processor_t> async_queue_;

        utils::mpsc_queue_t<core::error_code_t> code_queue_;
        code_callback_t code_callback_;
        std::thread code_worker_;
        std::atomic<bool> code_running_{false};
        std::mutex code_mutex_;
        std::condition_variable code_cv_;

        /**
         * @brief 启动纯错误码后台工作线程
         * @details 创建并启动 code_worker_ 线程，设置 code_running_ 标志
         */
        void start_code_worker_() noexcept;

        /**
         * @brief 纯错误码通道后台工作线程主循环
         * @details 从无锁队列出队 error_code_t，通过 code_callback_ 反查注册表并驱动插件
         */
        void code_worker_loop_() noexcept;

    public:
        /**
         * @brief 构造函数
         * @details 注入 error_context 回调与 error_code 回调。
         *          code_callback 为空时，error_code 通道不启动后台线程。
         * @note noexcept 风险：std::function 构造可能抛 std::bad_alloc，
         *       触发 std::terminate（仅在单例首次构造时存在）。
         */
        explicit async_notification_channel_t(notify_callback_t callback,
                                              code_callback_t code_callback = {}) noexcept
            : async_queue_(context_processor_t{std::move(callback)}),
              code_callback_(std::move(code_callback)) {
            if (code_callback_) {
                start_code_worker_();
            }
        }

        ~async_notification_channel_t() noexcept;

        async_notification_channel_t(const async_notification_channel_t&) = delete;
        async_notification_channel_t& operator=(const async_notification_channel_t&) = delete;
        async_notification_channel_t(async_notification_channel_t&&) = delete;
        async_notification_channel_t& operator=(async_notification_channel_t&&) = delete;

        /**
         * @brief 异步入队错误上下文通知
         * @details 将错误上下文副本推入后台队列，由工作线程异步处理。
         *          首次调用时自动启动后台工作线程。内存分配失败时记录日志并丢弃。
         */
        void enqueue_notification(const core::error_context_t& context) noexcept;

        /**
         * @brief 异步入队纯错误码通知（Lean 模式专用）
         * @details 通过无锁队列投递 error_code_t，调用方数纳秒完成。
         *          后台线程出队后反查注册表，构造轻量 error_context_t 并驱动插件。
         *          若 error_code 通道未启动（code_callback 为空），则静默丢弃。
         */
        void enqueue_code(core::error_code_t code) noexcept;

        /**
         * @brief 获取 error_context 队列待处理数量
         */
        [[nodiscard]] size_t pending_notifications() const noexcept {
            return async_queue_.size();
        }

        /**
         * @brief 获取 error_code 队列待处理数量
         * @details mpsc_queue_t 无 size() 接口，仅返回 0/1 指示是否有待处理项。
         *          仅供测试等待使用，不要求精确计数。
         */
        [[nodiscard]] size_t pending_codes() const noexcept {
            return code_queue_.empty() ? 0 : 1;
        }

        /**
         * @brief 设置 error_context 队列最大容量
         * @details 队列满时新通知将被丢弃（默认 0 = 无限制）
         */
        void set_max_queue_size(size_t max_size) noexcept {
            async_queue_.set_max_size(max_size);
        }

        /**
         * @brief 获取 error_context 队列最大容量
         */
        [[nodiscard]] size_t get_max_queue_size() const noexcept {
            return async_queue_.max_size();
        }
    };

}  // namespace error_system::plugin
