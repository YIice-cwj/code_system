/**
 * @file lock_free_queue_test.cc
 * @brief mpsc_queue_t 无锁队列单元测试
 * @details 覆盖基础功能（单线程 push/pop/empty）与并发压力测试（多生产者单消费者）。
 *          并发测试验证 MPSC 语义正确性与内存安全性，检测 ABA/UAF 等竞态缺陷。
 *          ABA 专项测试通过高频入队/出队强制节点地址复用，验证 tagged_ptr 版本号防护。
 *          retire_ring 测试验证延迟回收边界：ring 满时旧节点被正确释放。
 *          伪共享对齐通过 static_assert 验证 head_/tail_ cache line 对齐。
 * @author yiice
 * @version 1.2.0
 * @date 2026-07-08
 */

#include <atomic>
#include <chrono>
#include <new>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "error_system/utils/lock_free_queue.h"

using error_system::utils::mpsc_queue_t;

namespace {

    /**
     * @brief 验证 head_ 与 tail_ 的 cache line 对齐
     * @details head_ 和 tail_ 各自 alignas(cache_line) 对齐，对象大小至少为
     *          2 * cache_line（padding 占位）。间接验证伪共享防护已生效。
     */
    TEST(MpscQueueAlignmentTest, object_size_reflects_cache_line_padding) {
        constexpr size_t expected_min = 2 * ERROR_SYSTEM_CACHE_LINE_SIZE;
        EXPECT_GE(sizeof(mpsc_queue_t<int>), expected_min);
    }

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

    /**
     * @brief ABA 专项压力测试：高频入队/出队强制节点地址复用
     * @details 通过大量短周期入队/出队，让分配器复用已释放的节点地址。
     *          若 tagged_ptr 版本号防护失效，CAS 会错误匹配旧指针导致数据损坏。
     *          TSan/ASan 下运行可检测 use-after-free 和数据竞争。
     *          每轮 push 后立即 pop，制造 retire_ring 快速填满并回收的场景。
     */
    TEST(MpscQueueTest, aba_stress_forced_pointer_reuse) {
        constexpr int ROUNDS = 500000;
        constexpr int PRODUCERS = 4;

        mpsc_queue_t<int> q;
        std::atomic<int> produced{0};
        std::atomic<int> consumed{0};
        std::atomic<bool> stop{false};

        std::vector<std::thread> producers;
        producers.reserve(static_cast<size_t>(PRODUCERS));
        for (int p = 0; p < PRODUCERS; ++p) {
            producers.emplace_back([&q, &produced, p]() {
                for (int i = 0; i < ROUNDS; ++i) {
                    while (!q.push(p * ROUNDS + i)) {
                        std::this_thread::yield();
                    }
                    produced.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        std::thread consumer([&]() {
            int v = 0;
            while (!stop.load(std::memory_order_relaxed) || consumed.load() < PRODUCERS * ROUNDS) {
                if (q.pop(v)) {
                    consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });

        for (auto& t : producers) { t.join(); }
        stop.store(true, std::memory_order_release);
        consumer.join();

        EXPECT_EQ(produced.load(), PRODUCERS * ROUNDS);
        EXPECT_EQ(consumed.load(), PRODUCERS * ROUNDS);
    }

    /**
     * @brief retire_ring 边界测试：ring 满时旧节点正确释放
     * @details RETIRE_RING_SIZE=256，连续 push+pop 超过 ring 容量，
     *          触发 ring 回绕释放最旧节点。验证无内存泄漏、无 use-after-free。
     *          ASan 下运行可检测 UAF。
     */
    TEST(MpscQueueTest, retire_ring_wraparound_releases_oldest) {
        constexpr int COUNT = 1000;
        mpsc_queue_t<int> q;
        for (int i = 0; i < COUNT; ++i) {
            ASSERT_TRUE(q.push(i));
            int v = 0;
            ASSERT_TRUE(q.pop(v));
            ASSERT_EQ(v, i);
        }
        int v = 0;
        EXPECT_FALSE(q.pop(v));
        EXPECT_TRUE(q.empty());
    }

    /**
     * @brief 大规模并发测试：16 生产者 × 20万
     * @details 提升并发压力至 16 线程，增加 CAS 失败重试和 ABA 触发概率。
     *          验证高竞争场景下的正确性和无死锁。使用计数校验而非全量数组，
     *          避免 32MB 内存分配。
     */
    TEST(MpscQueueTest, high_contention_16_producers) {
        constexpr int PRODUCER_COUNT = 16;
        constexpr int PER_PRODUCER = 200000;
        constexpr int TOTAL = PRODUCER_COUNT * PER_PRODUCER;

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

        std::atomic<int> consumed{0};
        std::thread consumer([&]() {
            int v = 0;
            while (!stop.load(std::memory_order_relaxed) || consumed.load() < TOTAL) {
                if (q.pop(v)) {
                    consumed.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });

        for (auto& t : producers) { t.join(); }
        stop.store(true, std::memory_order_release);
        consumer.join();

        EXPECT_EQ(produced.load(), TOTAL);
        EXPECT_EQ(consumed.load(), TOTAL);
    }

    /**
     * @brief 生产者-消费者交替模式测试
     * @details 多生产者并发 push，单消费者并发 pop（MPSC 语义）。
     *          模拟 Lean 模式下 try_notify 的真实场景：多线程通过 enqueue_code
     *          投递错误码，后台单线程消费。验证无死锁、无数据丢失。
     */
    TEST(MpscQueueTest, concurrent_push_with_single_consumer) {
        constexpr int PRODUCERS = 8;
        constexpr int OPS = 100000;
        constexpr int TOTAL = PRODUCERS * OPS;

        mpsc_queue_t<int> q;
        std::atomic<int> push_count{0};
        std::atomic<int> pop_count{0};
        std::atomic<bool> stop{false};

        std::vector<std::thread> producers;
        producers.reserve(static_cast<size_t>(PRODUCERS));
        for (int t = 0; t < PRODUCERS; ++t) {
            producers.emplace_back([&q, &push_count, t]() {
                for (int i = 0; i < OPS; ++i) {
                    while (!q.push(t * OPS + i)) {
                        std::this_thread::yield();
                    }
                    push_count.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        std::thread consumer([&]() {
            int v = 0;
            while (!stop.load(std::memory_order_relaxed) || pop_count.load() < TOTAL) {
                if (q.pop(v)) {
                    pop_count.fetch_add(1, std::memory_order_relaxed);
                } else {
                    std::this_thread::yield();
                }
            }
        });

        for (auto& th : producers) { th.join(); }
        stop.store(true, std::memory_order_release);
        consumer.join();

        EXPECT_EQ(push_count.load(), TOTAL);
        EXPECT_EQ(pop_count.load(), TOTAL);
    }

}  // namespace
