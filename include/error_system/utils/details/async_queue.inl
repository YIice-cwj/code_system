#pragma once

/**
 * @file async_queue.inl
 * @brief async_queue_t 模板实现
 * @author yiice
 * @version 3.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <system_error>

#include "error_system/utils/log.h"

namespace error_system::utils {

template <typename T, typename Processor>
void async_queue_t<T, Processor>::stop_() noexcept {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_.load()) {
            return;
        }
        running_.store(false);
    }
    cv_.notify_all();
    if (worker_.joinable()) {
        try {
            worker_.join();
        } catch (const std::system_error&) {
            LOG_ERROR("[async_queue] stop_: worker_.join() threw std::system_error");
        }
    }
}

template <typename T, typename Processor>
void async_queue_t<T, Processor>::worker_loop_() noexcept {
    while (true) {
        value_type_t item;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] {
                return !queue_.empty() || !running_.load();
            });
            if (queue_.empty() && !running_.load()) {
                return;
            }
            item = std::move(queue_.front());
            queue_.pop();
        }
        try {
            processor_(item);
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[async_queue] processor std::bad_alloc caught and ignored");
        } catch (...) {
            LOG_ERROR("[async_queue] processor unknown exception caught and ignored");
        }
    }
}

template <typename T, typename Processor>
async_queue_t<T, Processor>::async_queue_t(processor_t processor) noexcept
    : processor_(std::move(processor)) {}

template <typename T, typename Processor>
async_queue_t<T, Processor>::~async_queue_t() noexcept {
    stop_();
}

template <typename T, typename Processor>
bool async_queue_t<T, Processor>::enqueue(value_type_t item) noexcept {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_.load()) {
            try {
                worker_ = std::thread(&async_queue_t::worker_loop_, this);
                running_.store(true);
            } catch (const std::system_error&) {
                LOG_ERROR("[async_queue] enqueue: failed to create worker thread");
                return false;
            }
        }
        if (max_size_ > 0 && queue_.size() >= max_size_) {
            return false;
        }
        try {
            queue_.push(std::move(item));
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[async_queue] enqueue: std::bad_alloc");
            return false;
        }
    }
    cv_.notify_one();
    return true;
}

template <typename T, typename Processor>
void async_queue_t<T, Processor>::set_max_size(size_t size) noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    max_size_ = size;
}

template <typename T, typename Processor>
size_t async_queue_t<T, Processor>::max_size() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_size_;
}

template <typename T, typename Processor>
size_t async_queue_t<T, Processor>::size() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

template <typename T, typename Processor>
bool async_queue_t<T, Processor>::empty() const noexcept {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

}  // namespace error_system::utils
