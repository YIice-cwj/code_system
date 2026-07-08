#pragma once

#include "error_system/utils/lock_free_queue.h"

#include <new>

namespace error_system::utils {

    template <typename T>
    mpsc_queue_t<T>::mpsc_queue_t() noexcept {
        for (size_t i = 0; i < MAX_HAZARDS; ++i) {
            hazards_[i].store(nullptr, std::memory_order_relaxed);
        }
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
        for (size_t i = 0; i < retired_count_; ++i) {
            std::unique_ptr<node_t> owned(retired_list_[i]);
        }
        retired_count_ = 0;
    }

    template <typename T>
    size_t mpsc_queue_t<T>::acquire_hazard_slot_(node_t* ptr) noexcept {
        for (size_t i = 0; i < MAX_HAZARDS; ++i) {
            node_t* expected = nullptr;
            if (hazards_[i].compare_exchange_strong(
                    expected, ptr,
                    std::memory_order_acq_rel,
                    std::memory_order_relaxed)) {
                return i;
            }
        }
        std::abort();
        return SIZE_MAX;
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

            const size_t slot = acquire_hazard_slot_(tail_ptr);

            if (tail != tail_.load(std::memory_order_acquire)) {
                hazards_[slot].store(nullptr, std::memory_order_release);
                continue;
            }

            node_t* next = tail_ptr->next.load(std::memory_order_acquire);

            if (tail != tail_.load(std::memory_order_acquire)) {
                hazards_[slot].store(nullptr, std::memory_order_release);
                continue;
            }

            if (next == nullptr) {
                if (tail_ptr->next.compare_exchange_weak(
                        next, raw,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                    hazards_[slot].store(nullptr, std::memory_order_release);
                    const packed_t new_tail = tagged_t::pack(raw, static_cast<typename tagged_t::tag_t>(tail_tag + 1));
                    tail_.compare_exchange_strong(
                        tail, new_tail,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                    return true;
                }
                hazards_[slot].store(nullptr, std::memory_order_release);
                continue;
            }

            hazards_[slot].store(nullptr, std::memory_order_release);
            const packed_t new_tail = tagged_t::pack(next, static_cast<typename tagged_t::tag_t>(tail_tag + 1));
            tail_.compare_exchange_strong(
                tail, new_tail,
                std::memory_order_release,
                std::memory_order_relaxed);
        }
    }

    template <typename T>
    void mpsc_queue_t<T>::retire_node_(node_t* node) noexcept {
        if (retired_count_ < RETIRE_THRESHOLD) {
            retired_list_[retired_count_++] = node;
            return;
        }
        scan_hazards_and_reclaim_();
        if (retired_count_ < RETIRE_THRESHOLD) {
            retired_list_[retired_count_++] = node;
        }
    }

    template <typename T>
    void mpsc_queue_t<T>::scan_hazards_and_reclaim_() noexcept {
        std::array<node_t*, MAX_HAZARDS> hazarded{};
        size_t haz_count = 0;
        for (size_t i = 0; i < MAX_HAZARDS; ++i) {
            node_t* h = hazards_[i].load(std::memory_order_acquire);
            if (h != nullptr) {
                hazarded[haz_count++] = h;
            }
        }

        size_t write_idx = 0;
        for (size_t read_idx = 0; read_idx < retired_count_; ++read_idx) {
            node_t* candidate = retired_list_[read_idx];
            bool is_hazarded = false;
            for (size_t i = 0; i < haz_count; ++i) {
                if (hazarded[i] == candidate) {
                    is_hazarded = true;
                    break;
                }
            }
            if (is_hazarded) {
                retired_list_[write_idx++] = candidate;
            } else {
                std::unique_ptr<node_t> owned(candidate);
            }
        }
        retired_count_ = write_idx;
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
