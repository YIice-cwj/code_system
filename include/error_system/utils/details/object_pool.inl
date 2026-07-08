#pragma once

/**
 * @file object_pool.inl
 * @brief object_pool_t<T> 模板实现
 * @details placement new 构造 + 显式析构保留内存 + ::operator delete 释放。
 *          构造失败时归还节点到空闲链表，避免内存泄漏。
 * @author yiice
 * @version 1.0.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief 析构空闲链表持有者
     * @details 线程退出时由 thread_local 自动调用。遍历链表释放所有节点内存。
     *          节点内存通过 ::operator delete 释放而非 delete，因为
     *          T 已在 release() 中被显式析构，再调用 delete 会
     *          触发 ~T() 的 double-destroy（double-free）。
     */
    template <typename T>
    object_pool_t<T>::free_list_holder_t::~free_list_holder_t() noexcept {
        while (head) {
            pooled_node_t* next = head->next;
            ::operator delete(head);
            head = next;
        }
        count = 0;
    }

    template <typename T>
    typename object_pool_t<T>::free_list_holder_t& object_pool_t<T>::tls_free_list_() noexcept {
        thread_local free_list_holder_t list;
        return list;
    }

    /**
     * @brief 从池获取对象并转发构造参数
     * @details 优先从空闲链表取节点，否则堆分配新节点。placement new 构造 T。
     *          构造失败（bad_alloc）时归还节点到链表，返回 nullptr。
     */
    template <typename T>
    template <typename... Args>
    T* object_pool_t<T>::acquire(Args&&... args) noexcept {
        free_list_holder_t& list = tls_free_list_();
        pooled_node_t* node = list.head;
        if (node) {
            list.head = node->next;
            --list.count;
        } else {
            try {
                node = new pooled_node_t;
            } catch (const std::bad_alloc&) {
                report_bad_alloc("object_pool", "acquire");
                return nullptr;
            }
        }
        node->next = nullptr;
        try {
            new (&node->object) T(std::forward<Args>(args)...);
            return &node->object;
        } catch (const std::bad_alloc&) {
            report_bad_alloc("object_pool", "acquire");
            node->next = list.head;
            list.head = node;
            ++list.count;
            return nullptr;
        }
    }

    /**
     * @brief 归还对象到池
     * @details 显式析构对象后，将节点放回空闲链表。链表超上限时释放回全局堆。
     *          使用 ::operator delete 释放内存，避免 double-destroy。
     */
    template <typename T>
    void object_pool_t<T>::release(T* obj) noexcept {
        if (obj == nullptr) {
            return;
        }
        obj->~T();
        pooled_node_t* node = reinterpret_cast<pooled_node_t*>(
            reinterpret_cast<char*>(obj) - offsetof(pooled_node_t, object));
        free_list_holder_t& list = tls_free_list_();
        if (list.count >= MAX_FREE) {
            ::operator delete(node);
            return;
        }
        node->next = list.head;
        list.head = node;
        ++list.count;
    }

}  // namespace error_system::utils
