#pragma once
#include <cstddef>
#include <new>
#include <utility>

#include "error_system/utils/bad_alloc_handler.h"

/**
 * @file object_pool.h
 * @brief 通用线程本地对象池
 * @details 为任意可默认构造类型 T 提供 thread_local 空闲链表复用，
 *          消除高频路径上的重复堆分配开销。
 *
 *          线程安全模型：
 *          - 池为 thread_local，每个线程独立空闲链表，无锁
 *          - acquire 后所有权转移给调用方（通过 pool_ptr_t<T> RAII 管理）
 *          - release 归还到当前线程的空闲链表；跨线程传递时归还到目标线程池
 *          - 空闲链表有上限（MAX_FREE），超限直接释放回全局堆
 *          - 线程退出时由 free_list_holder_t 析构释放所有空闲节点
 *
 *          节点布局：pooled_node_t = { T object; pooled_node_t* next; }
 *          acquire 时 placement new 构造 T；release 时显式析构保留内存。
 *          内存释放使用 ::operator delete，避免 double-destroy。
 *
 *          模板支持可变参数构造：acquire<Args...>(args...) 转发构造参数。
 * @author yiice
 * @version 1.0.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief 通用对象池模板
     * @tparam T 池化对象类型，必须可析构（析构函数 noexcept）
     * @details 线程本地空闲链表，复用对象内存避免重复堆分配。
     *          所有方法均为 noexcept，分配失败返回 nullptr。
     */
    template <typename T>
    class object_pool_t {
    public:
        /**
         * @brief 从当前线程池获取一个 T 对象
         * @details 空闲链表非空时取头节点并 placement new 构造；否则堆分配新节点。
         *          调用方通过 pool_ptr_t<T> 持有所有权，析构时自动归还。
         * @tparam Args 构造参数类型
         * @param args 转发给 T 构造函数的参数
         * @return 构造好的 T 指针，分配失败返回 nullptr
         */
        template <typename... Args>
        [[nodiscard]] static T* acquire(Args&&... args) noexcept;

        /**
         * @brief 归还 T 对象到当前线程池
         * @details 显式析构对象后，将内存放回空闲链表。空闲链表超上限时释放回全局堆。
         *          必须由 acquire 的同一线程（或 pool_ptr_t 析构所在线程）调用。
         * @param obj 待归还的 T 指针，可为 nullptr（空操作）
         */
        static void release(T* obj) noexcept;

    private:
        struct pooled_node_t {
            T object;
            pooled_node_t* next;
        };

        struct free_list_holder_t {
            pooled_node_t* head{nullptr};
            size_t count{0};
            ~free_list_holder_t() noexcept;
        };

        static constexpr size_t MAX_FREE = 64;

        [[nodiscard]] static free_list_holder_t& tls_free_list_() noexcept;
    };

    /**
     * @brief 池化对象的 RAII 智能指针
     * @tparam T 池化对象类型
     * @details 构造时持有裸指针，析构时归还到对象池。语义同 unique_ptr，仅移动。
     *          替代 std::unique_ptr<T>，避免 release 时调用 delete 析构+释放内存。
     */
    template <typename T>
    class pool_ptr_t {
    public:
        pool_ptr_t() noexcept = default;
        explicit pool_ptr_t(T* p) noexcept : ptr_(p) {}
        ~pool_ptr_t() noexcept { reset(); }

        pool_ptr_t(const pool_ptr_t&) = delete;
        pool_ptr_t& operator=(const pool_ptr_t&) = delete;
        pool_ptr_t(pool_ptr_t&& other) noexcept : ptr_(other.ptr_) { other.ptr_ = nullptr; }
        pool_ptr_t& operator=(pool_ptr_t&& other) noexcept {
            if (this != &other) {
                reset();
                ptr_ = other.ptr_;
                other.ptr_ = nullptr;
            }
            return *this;
        }

        [[nodiscard]] T& operator*() const noexcept { return *ptr_; }
        [[nodiscard]] T* operator->() const noexcept { return ptr_; }
        [[nodiscard]] T* get() const noexcept { return ptr_; }
        [[nodiscard]] explicit operator bool() const noexcept { return ptr_ != nullptr; }

        /**
         * @brief 释放所有权，返回裸指针
         * @return 持有的裸指针，调用方需自行管理生命周期
         */
        T* release() noexcept {
            T* p = ptr_;
            ptr_ = nullptr;
            return p;
        }

        /**
         * @brief 归还对象到池并置空
         */
        void reset() noexcept {
            if (ptr_) {
                object_pool_t<T>::release(ptr_);
                ptr_ = nullptr;
            }
        }

    private:
        T* ptr_{nullptr};
    };

}  // namespace error_system::utils

#include "error_system/utils/details/object_pool.inl"
