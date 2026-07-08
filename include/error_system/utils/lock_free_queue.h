#pragma once
#include <cstddef>
#include <cstdint>

#include <array>
#include <atomic>
#include <memory>
#include <type_traits>
#include <utility>

#include "error_system/utils/tagged_ptr.h"

/**
 * @file lock_free_queue.h
 * @brief MPSC 无锁队列
 * @details 多生产者单消费者无锁队列，基于 Michael-Scott 算法变种。
 *          使用 tagged_ptr_t 标记指针（48 位指针 + 16 位版本号）打包到
 *          64 位原子中，解决 ABA 问题。所有原子操作在 64 位平台上均为 lock-free。
 *          适用于高频错误通知、异步事件投递等无锁场景。
 * @author yiice
 * @version 1.1.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief MPSC 无锁队列
     * @details 基于侵入式链表的无锁队列，多线程 push 安全，单线程 pop。
     *          标记指针每次 CAS 成功后递增版本号，即使指针被复用，
     *          版本号不同也会导致 CAS 失败，彻底消除 ABA 问题。
     *          push 为 wait-free（有限步数内完成），pop 为 lock-free。
     * @tparam T 元素类型，要求可默认构造和可移动构造
     * @note 仅支持 64 位平台（指针 8 字节，地址空间 ≤ 48 位）
     */
    template <typename T>
    class mpsc_queue_t {
    public:
        using value_type_t = T;

    private:
        /**
         * @brief 链表节点
         * @details 持有元素数据与指向下一节点的原子指针。
         *          next 为 nullptr 表示链表尾部。
         */
        struct node_t {
            value_type_t data;
            std::atomic<node_t*> next;

            node_t() noexcept(std::is_nothrow_default_constructible_v<value_type_t>)
                : data{}, next{nullptr} {}

            explicit node_t(value_type_t v) noexcept(std::is_nothrow_move_constructible_v<value_type_t>)
                : data(std::move(v)), next{nullptr} {}
        };

        using tagged_t = tagged_ptr_t<node_t>;

    public:
        /**
         * @brief 打包后的标记指针类型
         * @details 低 48 位存储节点指针，高 16 位存储版本号。
         *          每次成功 CAS 后版本号递增，用于 ABA 检测。
         */
        using packed_t = typename tagged_t::packed_t;

    private:
        /**
         * @brief 延迟回收环大小
         * @details pop 弹出的节点不立即释放，而是进入 retire ring。
         *          当 ring 满时，最旧节点被释放。这为生产者提供了
         *          RETIRE_RING_SIZE 次 pop 的宽限期来完成对旧 tail 的访问，
         *          消除 push 与 pop 之间的 use-after-free 竞态。
         *          仅单消费者线程访问 retire ring，无需同步。
         */
        static constexpr size_t RETIRE_RING_SIZE = 256;

        std::atomic<packed_t> head_;
        std::atomic<packed_t> tail_;

        std::array<node_t*, RETIRE_RING_SIZE> retire_ring_{};
        size_t retire_pos_{0};

        /**
         * @brief 延迟回收节点
         * @details 将节点放入 retire ring，若 ring 已满则释放最旧节点。
         *          仅由单消费者 pop 调用，无需同步。
         * @param node 待回收的节点
         */
        void retire_node_(node_t* node) noexcept;

    public:
        /**
         * @brief 构造函数
         * @details 创建哑节点，head 与 tail 均指向哑节点，版本号为 0。
         * @note noexcept 风险：哑节点内存分配可能抛 std::bad_alloc，
         *       触发 std::terminate。仅在队列构造时存在，可接受。
         */
        mpsc_queue_t() noexcept;

        /**
         * @brief 析构函数
         * @details 弹出并释放所有剩余节点，最后释放哑节点。
         *          仅在无并发访问时调用（队列生命周期结束）。
         */
        ~mpsc_queue_t() noexcept;

        mpsc_queue_t(const mpsc_queue_t&) = delete;
        mpsc_queue_t& operator=(const mpsc_queue_t&) = delete;
        mpsc_queue_t(mpsc_queue_t&&) = delete;
        mpsc_queue_t& operator=(mpsc_queue_t&&) = delete;

        /**
         * @brief 入队（多生产者安全）
         * @details 创建新节点并链接到链表尾部，通过 CAS 推进 tail。
         *          多线程并发 push 时，失败者自旋重试直至成功。
         * @param value 待入队元素（移动语义）
         * @return bool true=成功，false=内存分配失败
         */
        bool push(value_type_t value) noexcept;

        /**
         * @brief 出队（仅单消费者调用）
         * @details 从链表头部弹出元素，通过 CAS 推进 head 并释放旧节点。
         * @param out 输出参数，接收弹出元素
         * @return bool true=成功，false=队列空
         */
        bool pop(value_type_t& out) noexcept;

        /**
         * @brief 检查队列是否为空
         * @details 弱一致性，仅用于观察，不用于并发决策。
         * @return bool true=队列空
         */
        [[nodiscard]] bool empty() const noexcept;
    };

}  // namespace error_system::utils

#include "error_system/utils/details/lock_free_queue.inl"
