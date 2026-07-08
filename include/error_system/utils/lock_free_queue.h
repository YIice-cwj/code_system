#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <array>
#include <atomic>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "error_system/utils/tagged_ptr.h"

#if defined(__cpp_lib_hardware_interference_size) && __cpp_lib_hardware_interference_size >= 201707L
    #define ERROR_SYSTEM_CACHE_LINE_SIZE std::hardware_destructive_interference_size
#else
    #define ERROR_SYSTEM_CACHE_LINE_SIZE 64
#endif

/**
 * @file lock_free_queue.h
 * @brief MPSC 无锁队列
 * @details 多生产者单消费者无锁队列，基于 Michael-Scott 算法变种。
 *          使用 tagged_ptr_t 标记指针（48 位指针 + 16 位版本号）打包到
 *          64 位原子中，解决 ABA 问题。所有原子操作在 64 位平台上均为 lock-free。
 *          head_ 与 tail_ 采用 cache line 对齐（alignas），消除生产者/消费者伪共享。
 *          节点回收采用 hazard pointer 机制：生产者 push 时通过 hazard slot
 *          保护正在访问的 tail 节点，消费者 retire 时扫描所有 hazard slot，
 *          仅回收未被保护的节点，彻底消除固定大小 retire ring 的 UAF 风险。
 *          适用于高频错误通知、异步事件投递等无锁场景。
 * @author yiice
 * @version 1.2.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief MPSC 无锁队列
     * @details 基于侵入式链表的无锁队列，多线程 push 安全，单线程 pop。
     *          标记指针每次 CAS 成功后递增版本号，即使指针被复用，
     *          版本号不同也会导致 CAS 失败，彻底消除 ABA 问题。
     *          push 为 wait-free（有限步数内完成），pop 为 lock-free。
     *          节点回收使用 hazard pointer：生产者发布正在访问的 tail 节点指针，
     *          消费者回收前扫描所有 hazard slot，被保护的节点延迟回收。
     * @tparam T 元素类型，要求可默认构造和可移动构造
     * @note 仅支持 64 位平台（指针 8 字节，地址空间 ≤ 48 位）
     * @note MAX_HAZARDS 限制最大并发生产者线程数（默认 64），超出将 abort
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
         * @brief 最大 hazard slot 数（即最大并发生产者线程数）
         */
        static constexpr size_t MAX_HAZARDS = 64;

        /**
         * @brief 延迟回收阈值
         * @details retired_list 达到此数量时触发 hazard scan，回收未被保护的节点。
         *          经典 hazard paper 建议 = 2 * 线程数。
         */
        static constexpr size_t RETIRE_THRESHOLD = 2 * MAX_HAZARDS;

        alignas(ERROR_SYSTEM_CACHE_LINE_SIZE) std::atomic<packed_t> head_;
        alignas(ERROR_SYSTEM_CACHE_LINE_SIZE) std::atomic<packed_t> tail_;

        /**
         * @brief hazard pointer slot 数组
         * @details 每个生产者线程 push 时 CAS 占用一个 slot（nullptr -> tail_ptr），
         *          访问完成后释放（store nullptr）。消费者回收前扫描所有 slot，
         *          被引用的节点延迟回收。slot 值为 nullptr 表示空闲。
         */
        std::array<std::atomic<node_t*>, MAX_HAZARDS> hazards_{};

        /**
         * @brief 延迟回收列表（仅消费者线程访问，无需同步）
         */
        std::array<node_t*, RETIRE_THRESHOLD> retired_list_{};
        size_t retired_count_{0};

        /**
         * @brief 获取 hazard slot
         * @details 线性扫描 hazards_ 数组，CAS nullptr -> ptr 占用首个空闲 slot。
         *          每次调用均扫描（push 临界区短，slot 快速释放，竞争低）。
         * @param ptr 要保护的节点指针
         * @return slot 索引；无空闲 slot 时 abort
         */
        size_t acquire_hazard_slot_(node_t* ptr) noexcept;

        /**
         * @brief 延迟回收节点
         * @details 将节点加入 retired_list，达到阈值时扫描 hazard slot
         *          回收未被保护的节点。仅由单消费者 pop 调用，无需同步。
         * @param node 待回收的节点
         */
        void retire_node_(node_t* node) noexcept;

        /**
         * @brief 扫描 hazard slot 并回收安全节点
         * @details 收集所有非空 hazard 指针，retired_list 中未被引用的节点释放，
         *          被引用的保留到下一轮。仅消费者线程调用，无需同步。
         */
        void scan_hazards_and_reclaim_() noexcept;

    public:
        /**
         * @brief 构造函数
         * @details 创建哑节点，head 与 tail 均指向哑节点，版本号为 0。
         *          初始化所有 hazard slot 为 nullptr。
         * @note noexcept 风险：哑节点内存分配可能抛 std::bad_alloc，
         *       触发 std::terminate。仅在队列构造时存在，可接受。
         */
        mpsc_queue_t() noexcept;

        /**
         * @brief 析构函数
         * @details 弹出并释放所有剩余节点，最后释放哑节点和 retired_list。
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
         *          push 通过 hazard pointer 保护正在访问的 tail 节点：
         *          设置 hazard -> 双检查 tail 未变 -> 安全访问 tail_ptr->next。
         *          多线程并发 push 时，失败者自旋重试直至成功。
         * @param value 待入队元素（移动语义）
         * @return bool true=成功，false=内存分配失败
         */
        bool push(value_type_t value) noexcept;

        /**
         * @brief 出队（仅单消费者调用）
         * @details 从链表头部弹出元素，通过 CAS 推进 head 并回收旧节点。
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
