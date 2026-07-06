#include "error_system/plugin/async_notification_channel.h"

/**
 * @file async_notification_channel.cc
 * @brief 异步通知通道实现
 * @details 提供错误上下文的异步入队能力，封装 async_queue_t 的入队细节与异常处理。
 *          出队上下文的转发逻辑由 context_processor_t 在头文件内联实现，
 *          本文件仅包含可能抛出 std::bad_alloc 的入队路径（大型函数，分离至 .cc）。
 * @author yiice
 * @version 3.0.0
 * @date 2026-06-27
 * @copyright Copyright (c) 2026
 */

#include <new>

namespace error_system::plugin {

    /**
     * @brief 异步入队错误通知
     * @details 将错误上下文副本推入后台队列，由工作线程异步处理。
     *          首次调用时自动启动后台工作线程。内存分配失败时记录日志并丢弃。
     * @param context 错误上下文
     */
    void async_notification_channel_t::enqueue_notification(const core::error_context_t& context) noexcept {
        try {
            auto copy = std::make_shared<core::error_context_t>(context.clone());
            async_queue_.enqueue(std::move(copy));
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr,
                         "[async_notification_channel] enqueue_notification failed to allocate memory\n");
        }
    }

}  // namespace error_system::plugin
