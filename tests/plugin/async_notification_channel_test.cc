/**
 * @file async_notification_channel_test.cc
 * @brief async_notification_channel_t 单元测试
 * @details 覆盖 enqueue_notification / pending_notifications / set_max_queue_size /
 *          get_max_queue_size 全部公共 API；验证后台线程异步消费、背压丢弃、
 *          析构自动停止线程、空回调安全。
 */

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include "error_system/core/error_context.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/plugin/async_notification_channel.h"

using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::located_code_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::plugin::async_notification_channel_t;

namespace {

    /** @brief 构造测试用错误上下文 */
    error_context_t make_context(uint16_t number = 1) {
        const error_code_t code(error_level_t::error, system_domain_t::application,
                                subsystem_id_t{1}, module_id_t{1}, error_number_t{number});
        return error_context_t{located_code_t{code}, "异步通道测试"};
    }

    /** @brief 同步等待条件成立的工具 */
    bool wait_for(std::atomic<int>& counter, int expected, int timeout_ms = 2000) {
        for (int i = 0; i < timeout_ms / 10; ++i) {
            if (counter.load() >= expected) { return true; }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return counter.load() >= expected;
    }

}  // namespace

TEST(AsyncNotificationChannelTest, enqueue_invokes_callback_asynchronously) {
    std::atomic<int> call_count{0};
    std::mutex mtx;
    std::condition_variable cv;
    async_notification_channel_t channel([&](const error_context_t&) {
        std::lock_guard<std::mutex> lk(mtx);
        ++call_count;
        cv.notify_one();
    });
    channel.enqueue_notification(make_context(1));
    channel.enqueue_notification(make_context(2));
    channel.enqueue_notification(make_context(3));

    std::unique_lock<std::mutex> lk(mtx);
    EXPECT_TRUE(cv.wait_for(lk, std::chrono::seconds(2), [&] { return call_count.load() >= 3; }));
    EXPECT_EQ(call_count.load(), 3);
}

TEST(AsyncNotificationChannelTest, pending_notifications_reflects_queue_size) {
    std::atomic<int> call_count{0};
    std::mutex mtx;
    std::condition_variable cv;
    async_notification_channel_t channel([&](const error_context_t&) {
        std::lock_guard<std::mutex> lk(mtx);
        ++call_count;
        cv.notify_one();
    });
    EXPECT_EQ(channel.pending_notifications(), 0u);

    channel.enqueue_notification(make_context(1));
    EXPECT_TRUE(wait_for(call_count, 1));
    EXPECT_EQ(channel.pending_notifications(), 0u);
}

TEST(AsyncNotificationChannelTest, set_max_queue_size_enforces_backpressure) {
    std::atomic<int> call_count{0};
    std::mutex block_mtx;
    block_mtx.lock();
    async_notification_channel_t channel([&](const error_context_t&) {
        ++call_count;
        std::lock_guard<std::mutex> lk(block_mtx);
    });
    channel.set_max_queue_size(1);

    channel.enqueue_notification(make_context(1));
    channel.enqueue_notification(make_context(2));
    channel.enqueue_notification(make_context(3));

    block_mtx.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_LE(call_count.load(), 2);
    EXPECT_EQ(channel.get_max_queue_size(), 1u);
}

TEST(AsyncNotificationChannelTest, get_max_queue_size_default_unlimited) {
    async_notification_channel_t channel([](const error_context_t&) {});
    EXPECT_EQ(channel.get_max_queue_size(), 0u);
}

TEST(AsyncNotificationChannelTest, null_callback_is_safe) {
    async_notification_channel_t channel(nullptr);
    channel.enqueue_notification(make_context(1));
    EXPECT_NO_THROW(channel.enqueue_notification(make_context(2)));
}

TEST(AsyncNotificationChannelTest, destructor_stops_worker_thread) {
    std::atomic<int> call_count{0};
    {
        async_notification_channel_t channel([&](const error_context_t&) {
            ++call_count;
        });
        channel.enqueue_notification(make_context(1));
        EXPECT_TRUE(wait_for(call_count, 1));
    }
    const int snapshot = call_count.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_EQ(call_count.load(), snapshot);
}

TEST(AsyncNotificationChannelTest, enqueue_multiple_contexts_processes_all) {
    constexpr int N = 100;
    std::atomic<int> call_count{0};
    async_notification_channel_t channel([&](const error_context_t&) {
        ++call_count;
    });

    for (int i = 0; i < N; ++i) {
        channel.enqueue_notification(make_context(static_cast<uint16_t>(i + 1)));
    }

    EXPECT_TRUE(wait_for(call_count, N, 5000));
    EXPECT_EQ(call_count.load(), N);
}

/**
 * @brief enqueue_code 唤醒及时性测试
 * @details 验证无锁队列 push 后 notify_one 能及时唤醒 worker 线程，
 *          不依赖 100ms 超时兜底。若发生 lost wakeup，延迟会接近 100ms。
 *          阈值设为 50ms，远小于超时兜底，确保唤醒路径正常。
 */
TEST(AsyncNotificationChannelTest, enqueue_code_wakes_worker_promptly) {
    std::atomic<int> call_count{0};
    std::atomic<bool> ready{false};

    async_notification_channel_t channel(
        [](const error_context_t&) {},
        [&](error_code_t) {
            call_count.fetch_add(1, std::memory_order_relaxed);
            ready.store(true, std::memory_order_release);
        });

    const error_code_t code(error_level_t::error, system_domain_t::application,
                            subsystem_id_t{1}, module_id_t{1}, error_number_t{1});

    ready.store(false);
    channel.enqueue_code(code);

    const auto start = std::chrono::steady_clock::now();
    while (!ready.load(std::memory_order_acquire) &&
           std::chrono::steady_clock::now() - start < std::chrono::milliseconds(200)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start);

    EXPECT_TRUE(ready.load()) << "worker 未在 200ms 内唤醒";
    EXPECT_LT(elapsed.count(), 50)
        << "唤醒延迟 " << elapsed.count() << "ms，疑似 lost wakeup（依赖超时兜底）";
    EXPECT_GE(call_count.load(), 1);
}

/**
 * @brief enqueue_code 并发压力测试
 * @details 多线程并发 enqueue_code，验证无锁队列在高并发下的正确性。
 *          所有 code 都应被 worker 消费，无丢失。
 */
TEST(AsyncNotificationChannelTest, concurrent_enqueue_code_no_loss) {
    constexpr int THREADS = 8;
    constexpr int PER_THREAD = 10000;
    constexpr int TOTAL = THREADS * PER_THREAD;

    std::atomic<int> call_count{0};
    async_notification_channel_t channel(
        [](const error_context_t&) {},
        [&](error_code_t) {
            call_count.fetch_add(1, std::memory_order_relaxed);
        });

    const error_code_t code(error_level_t::error, system_domain_t::application,
                            subsystem_id_t{1}, module_id_t{1}, error_number_t{1});

    std::vector<std::thread> threads;
    threads.reserve(static_cast<size_t>(THREADS));
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&channel, code]() {
            for (int i = 0; i < PER_THREAD; ++i) {
                channel.enqueue_code(code);
            }
        });
    }
    for (auto& th : threads) { th.join(); }

    EXPECT_TRUE(wait_for(call_count, TOTAL, 10000));
    EXPECT_EQ(call_count.load(), TOTAL);
}
