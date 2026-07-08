/**
 * @file lock_free_queue_test.cc
 * @brief mpsc_queue_t 无锁队列单元测试
 * @details 覆盖基础功能（单线程 push/pop/empty）与并发压力测试（多生产者单消费者）。
 *          并发测试验证 MPSC 语义正确性与内存安全性，检测 ABA/UAF 等竞态缺陷。
 */

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "error_system/utils/lock_free_queue.h"

using error_system::utils::mpsc_queue_t;

namespace {

    /**
     * @brief 基础入队出队验证
     */
    TEST(MpscQueueTest, push_pop_single_thread) {
        mpsc_queue_t<int> q;
        EXPECT_TRUE(q.empty());

        EXPECT_TRUE(q.push(1));
        EXPECT_TRUE(q.push(2));
        EXPECT_TRUE(q.push(3));
        EXPECT_FALSE(q.empty());

        int v = 0;
        EXPECT_TRUE(q.pop(v));
        EXPECT_EQ(v, 1);
        EXPECT_TRUE(q.pop(v));
        EXPECT_EQ(v, 2);
        EXPECT_TRUE(q.pop(v));
        EXPECT_EQ(v, 3);
        EXPECT_FALSE(q.pop(v));
        EXPECT_TRUE(q.empty());
    }

    /**
     * @brief FIFO 顺序验证
     */
    TEST(MpscQueueTest, fifo_order_preserved) {
        mpsc_queue_t<int> q;
        constexpr int N = 1000;
        for (int i = 0; i < N; ++i) {
            EXPECT_TRUE(q.push(i));
        }
        int v = 0;
        for (int i = 0; i < N; ++i) {
            EXPECT_TRUE(q.pop(v));
            EXPECT_EQ(v, i);
        }
        EXPECT_FALSE(q.pop(v));
    }

    /**
     * @brief 并发压力测试：多生产者单消费者
     * @details 每个生产者推入 [0, PER_PRODUCER) 范围内的值，
     *          消费者统计所有收到的值，验证总数与各生产者贡献数。
     */
    TEST(MpscQueueTest, concurrent_mpsc_stress) {
        constexpr int PRODUCER_COUNT = 4;
        constexpr int PER_PRODUCER = 100000;
        constexpr int TOTAL_INT = PRODUCER_COUNT * PER_PRODUCER;
        constexpr size_t TOTAL = static_cast<size_t>(TOTAL_INT);

        mpsc_queue_t<int> q;
        std::atomic<int> produced{0};
        std::atomic<bool> stop{false};

        std::vector<std::thread> producers;
        producers.reserve(static_cast<size_t>(PRODUCER_COUNT));
        for (int p = 0; p < PRODUCER_COUNT; ++p) {
            producers.emplace_back([&q, &produced, p]() {
                for (int i = 0; i < PER_PRODUCER; ++i) {
                    while (!q.push(p * PER_PRODUCER + i)) {
                        std::this_thread::yield();
                    }
                    produced.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        std::vector<int> received(TOTAL, 0);
        int total_consumed = 0;
        std::thread consumer([&]() {
            int v = 0;
            while (!stop.load(std::memory_order_relaxed) || total_consumed < TOTAL_INT) {
                if (q.pop(v)) {
                    ASSERT_GE(v, 0);
                    ASSERT_LT(v, TOTAL_INT);
                    received[static_cast<size_t>(v)] = 1;
                    ++total_consumed;
                } else {
                    std::this_thread::yield();
                }
            }
        });

        for (auto& t : producers) { t.join(); }
        stop.store(true, std::memory_order_release);
        consumer.join();

        EXPECT_EQ(total_consumed, TOTAL_INT);
        EXPECT_EQ(produced.load(), TOTAL_INT);
        for (size_t i = 0; i < TOTAL; ++i) {
            EXPECT_EQ(received[i], 1) << "缺失元素: " << i;
        }
    }

}  // namespace
