#pragma once

#include "error_system/utils/lock_free_queue.h"

#include <new>

namespace error_system::utils {

    template <typename T>
    mpsc_queue_t<T>::mpsc_queue_t() noexcept {
        auto dummy = std::make_unique<node_t>();
        const packed_t init = tagged_t::pack(dummy.get(), 0);
        head_.store(init, std::memory_order_relaxed);
        tail_.store(init, std::memory_order_relaxed);
        dummy.release();
    }

    template <typename T>
    mpsc_queue_t<T>::~mpsc_queue_t() noexcept {
        node_t* curr = tagged_t::get_ptr(head_.load(std::memory_order_acquire));
        while (curr != nullptr) {
            node_t* next = curr->next.load(std::memory_order_acquire);
            std::unique_ptr<node_t> owned(curr);
            curr = next;
        }
        for (size_t i = 0; i < RETIRE_RING_SIZE; ++i) {
            if (retire_ring_[i] != nullptr) {
                std::unique_ptr<node_t> owned(retire_ring_[i]);
                retire_ring_[i] = nullptr;
            }
        }
    }

    template <typename T>
    bool mpsc_queue_t<T>::push(value_type_t value) noexcept {
        std::unique_ptr<node_t> node;
        try {
            node = std::make_unique<node_t>(std::move(value));
        } catch (const std::bad_alloc&) {
            return false;
        }
        node->next.store(nullptr, std::memory_order_relaxed);
        node_t* raw = node.release();

        while (true) {
            packed_t tail = tail_.load(std::memory_order_acquire);
            node_t* tail_ptr = tagged_t::get_ptr(tail);
            const typename tagged_t::tag_t tail_tag = tagged_t::get_tag(tail);
            node_t* next = tail_ptr->next.load(std::memory_order_acquire);

            if (tail != tail_.load(std::memory_order_acquire)) {
                continue;
            }

            if (next == nullptr) {
                if (tail_ptr->next.compare_exchange_weak(
                        next, raw,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                    const packed_t new_tail = tagged_t::pack(raw, static_cast<typename tagged_t::tag_t>(tail_tag + 1));
                    tail_.compare_exchange_strong(
                        tail, new_tail,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                    return true;
                }
            } else {
                const packed_t new_tail = tagged_t::pack(next, static_cast<typename tagged_t::tag_t>(tail_tag + 1));
                tail_.compare_exchange_strong(
                    tail, new_tail,
                    std::memory_order_release,
                    std::memory_order_relaxed);
            }
        }
    }

    template <typename T>
    void mpsc_queue_t<T>::retire_node_(node_t* node) noexcept {
        node_t* old = retire_ring_[retire_pos_];
        retire_ring_[retire_pos_] = node;
        retire_pos_ = (retire_pos_ + 1) % RETIRE_RING_SIZE;
        if (old != nullptr) {
            std::unique_ptr<node_t> owned(old);
        }
    }

    template <typename T>
    bool mpsc_queue_t<T>::pop(value_type_t& out) noexcept {
        while (true) {
            packed_t head = head_.load(std::memory_order_acquire);
            packed_t tail = tail_.load(std::memory_order_acquire);
            node_t* head_ptr = tagged_t::get_ptr(head);
            const typename tagged_t::tag_t head_tag = tagged_t::get_tag(head);
            node_t* next = head_ptr->next.load(std::memory_order_acquire);

            if (head != head_.load(std::memory_order_acquire)) {
                continue;
            }

            if (head_ptr == tagged_t::get_ptr(tail)) {
                if (next == nullptr) {
                    return false;
                }
                const typename tagged_t::tag_t tail_tag = tagged_t::get_tag(tail);
                const packed_t new_tail = tagged_t::pack(next, static_cast<typename tagged_t::tag_t>(tail_tag + 1));
                tail_.compare_exchange_strong(
                    tail, new_tail,
                    std::memory_order_release,
                    std::memory_order_relaxed);
            } else {
                out = std::move(next->data);
                const packed_t new_head = tagged_t::pack(next, static_cast<typename tagged_t::tag_t>(head_tag + 1));
                if (head_.compare_exchange_strong(
                        head, new_head,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                    retire_node_(head_ptr);
                    return true;
                }
            }
        }
    }

    template <typename T>
    bool mpsc_queue_t<T>::empty() const noexcept {
        const packed_t head = head_.load(std::memory_order_acquire);
        const packed_t tail = tail_.load(std::memory_order_acquire);
        if (tagged_t::get_ptr(head) != tagged_t::get_ptr(tail)) {
            return false;
        }
        return tagged_t::get_ptr(head)->next.load(std::memory_order_acquire) == nullptr;
    }

}  // namespace error_system::utils
