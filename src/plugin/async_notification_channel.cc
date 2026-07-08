#include "error_system/plugin/async_notification_channel.h"

/**
 * @file async_notification_channel.cc
 * @brief 异步通知通道实现
 * @details 提供 error_context 异步入队与 error_code 无锁入队两条路径。
 *          error_code 路径使用 mpsc_queue_t 无锁队列，后台线程通过
 *          condition_variable 唤醒，出队后回调注入的 code_callback。
 * @author yiice
 * @version 4.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <new>

#include "error_system/utils/log.h"

namespace error_system::plugin {

    /**
     * @brief 异步入队错误上下文（克隆后入队，避免引用悬垂）
     * @param context 错误上下文
     */
    void async_notification_channel_t::enqueue_notification(const core::error_context_t& context) noexcept {
        try {
            auto copy = std::make_shared<core::error_context_t>(context.clone());
            async_queue_.enqueue(std::move(copy));
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[async_notification_channel] enqueue_notification failed to allocate memory");
        }
    }

    /**
     * @brief 入队错误码（无锁队列 + condition_variable 通知）
     * @param code 错误码
     */
    void async_notification_channel_t::enqueue_code(core::error_code_t code) noexcept {
        if (!code_callback_) {
            return;
        }
        code_queue_.push(code);
        {
            std::lock_guard<std::mutex> lk(code_mutex_);
        }
        code_cv_.notify_one();
    }

    /**
     * @brief 启动后台 code worker 线程
     */
    void async_notification_channel_t::start_code_worker_() noexcept {
        code_running_.store(true, std::memory_order_release);
        try {
            code_worker_ = std::thread([this]() { code_worker_loop_(); });
        } catch (const std::system_error& e) {
            code_running_.store(false, std::memory_order_release);
            LOG_ERROR("[async_notification_channel] failed to start code worker thread: {}", e.what());
        }
    }

    /**
     * @brief code worker 主循环：出队错误码并调用回调
     * @details 使用 condition_variable 等待，支持取消令牌停止
     */
    void async_notification_channel_t::code_worker_loop_() noexcept {
        while (true) {
            core::error_code_t code;
            while (code_queue_.pop(code)) {
                if (code_callback_) {
                    code_callback_(code);
                }
            }
            std::unique_lock<std::mutex> lk(code_mutex_);
            code_cv_.wait_for(lk, std::chrono::milliseconds(100), [this] {
                return !code_queue_.empty() || !code_running_.load(std::memory_order_relaxed);
            });
            if (!code_running_.load(std::memory_order_relaxed) && code_queue_.empty()) {
                break;
            }
        }
    }

    async_notification_channel_t::~async_notification_channel_t() noexcept {
        if (code_worker_.joinable()) {
            code_running_.store(false, std::memory_order_release);
            code_cv_.notify_all();
            code_worker_.join();
        }
    }

}  // namespace error_system::plugin
