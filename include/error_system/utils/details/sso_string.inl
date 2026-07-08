#pragma once

#include <cstring>
#include <new>
#include <string>
#include <string_view>
#include <utility>

#include "error_system/utils/bad_alloc_handler.h"

/**
 * @file sso_string.inl
 * @brief sso_string_t 模板实现
 * @details 48 字节内联缓冲区 + 堆回退。placement new + ::operator delete 管理堆内存。
 *          所有方法 noexcept，bad_alloc 内部 try-catch 处理。
 * @author yiice
 * @version 1.0.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    inline void sso_string_t::set_inline_(const char* s, size_t len) noexcept {
        if (len > SSO_CAPACITY) {
            set_heap_(s, len);
            return;
        }
        free_heap_();
        if (len > 0 && s != nullptr) {
            std::memcpy(storage_.inline_data, s, len);
        }
        storage_.inline_data[len] = '\0';
        size_ = len;
        is_heap_ = false;
    }

    inline void sso_string_t::set_heap_(const char* s, size_t len) noexcept {
        char* new_ptr = nullptr;
        try {
            new_ptr = static_cast<char*>(::operator new(len + 1));
        } catch (const std::bad_alloc&) {
            report_bad_alloc("sso_string", "set_heap_");
            free_heap_();
            if (len <= SSO_CAPACITY) {
                if (len > 0 && s != nullptr) {
                    std::memcpy(storage_.inline_data, s, len);
                }
                storage_.inline_data[len] = '\0';
                size_ = len;
                is_heap_ = false;
            } else {
                size_ = 0;
                is_heap_ = false;
                storage_.inline_data[0] = '\0';
            }
            return;
        }
        if (len > 0 && s != nullptr) {
            std::memcpy(new_ptr, s, len);
        }
        new_ptr[len] = '\0';
        free_heap_();
        storage_.heap.ptr = new_ptr;
        storage_.heap.capacity = len + 1;
        size_ = len;
        is_heap_ = true;
    }

    inline void sso_string_t::free_heap_() noexcept {
        if (is_heap_ && storage_.heap.ptr != nullptr) {
            ::operator delete(storage_.heap.ptr);
            storage_.heap.ptr = nullptr;
            storage_.heap.capacity = 0;
        }
        is_heap_ = false;
    }

    inline const char* sso_string_t::raw_data_() const noexcept {
        return is_heap_ ? storage_.heap.ptr : storage_.inline_data;
    }

    inline sso_string_t::sso_string_t(const char* s) noexcept {
        if (s != nullptr) {
            set_inline_(s, std::strlen(s));
        }
    }

    inline sso_string_t::sso_string_t(std::string_view sv) noexcept {
        set_inline_(sv.data(), sv.size());
    }

    inline sso_string_t::sso_string_t(const std::string& s) noexcept {
        set_inline_(s.data(), s.size());
    }

    inline sso_string_t::sso_string_t(std::string&& s) noexcept {
        set_inline_(s.data(), s.size());
    }

    inline sso_string_t::sso_string_t(const sso_string_t& other) noexcept {
        set_inline_(other.raw_data_(), other.size_);
    }

    inline sso_string_t& sso_string_t::operator=(const sso_string_t& other) noexcept {
        if (this != &other) {
            set_inline_(other.raw_data_(), other.size_);
        }
        return *this;
    }

    inline sso_string_t::sso_string_t(sso_string_t&& other) noexcept {
        if (other.is_heap_) {
            storage_.heap.ptr = other.storage_.heap.ptr;
            storage_.heap.capacity = other.storage_.heap.capacity;
            size_ = other.size_;
            is_heap_ = true;
            other.storage_.heap.ptr = nullptr;
            other.is_heap_ = false;
            other.size_ = 0;
        } else {
            std::memcpy(storage_.inline_data, other.storage_.inline_data, other.size_ + 1);
            size_ = other.size_;
            other.size_ = 0;
            other.storage_.inline_data[0] = '\0';
        }
    }

    inline sso_string_t& sso_string_t::operator=(sso_string_t&& other) noexcept {
        if (this != &other) {
            free_heap_();
            if (other.is_heap_) {
                storage_.heap.ptr = other.storage_.heap.ptr;
                storage_.heap.capacity = other.storage_.heap.capacity;
                size_ = other.size_;
                is_heap_ = true;
                other.storage_.heap.ptr = nullptr;
                other.is_heap_ = false;
                other.size_ = 0;
            } else {
                std::memcpy(storage_.inline_data, other.storage_.inline_data, other.size_ + 1);
                size_ = other.size_;
                is_heap_ = false;
                other.size_ = 0;
                other.storage_.inline_data[0] = '\0';
            }
        }
        return *this;
    }

    inline sso_string_t& sso_string_t::operator=(const char* s) noexcept {
        if (s == nullptr) {
            clear();
        } else {
            set_inline_(s, std::strlen(s));
        }
        return *this;
    }

    inline sso_string_t& sso_string_t::operator=(std::string_view sv) noexcept {
        set_inline_(sv.data(), sv.size());
        return *this;
    }

    inline sso_string_t& sso_string_t::operator=(const std::string& s) noexcept {
        set_inline_(s.data(), s.size());
        return *this;
    }

    inline sso_string_t& sso_string_t::operator=(std::string&& s) noexcept {
        set_inline_(s.data(), s.size());
        return *this;
    }

    inline const char* sso_string_t::c_str() const noexcept {
        return raw_data_();
    }

    inline const char* sso_string_t::data() const noexcept {
        return raw_data_();
    }

    inline sso_string_t::operator std::string_view() const noexcept {
        return std::string_view(raw_data_(), size_);
    }

    inline std::string sso_string_t::to_std_string() const noexcept {
        try {
            return std::string(raw_data_(), size_);
        } catch (const std::bad_alloc&) {
            report_bad_alloc("sso_string", "to_std_string");
            return std::string{};
        }
    }

    inline sso_string_t& sso_string_t::operator+=(std::string_view sv) noexcept {
        if (sv.empty()) {
            return *this;
        }
        const size_t new_size = size_ + sv.size();
        if (!is_heap_ && new_size <= SSO_CAPACITY) {
            std::memcpy(storage_.inline_data + size_, sv.data(), sv.size());
            storage_.inline_data[new_size] = '\0';
            size_ = new_size;
            return *this;
        }
        char* new_ptr = nullptr;
        try {
            new_ptr = static_cast<char*>(::operator new(new_size + 1));
        } catch (const std::bad_alloc&) {
            report_bad_alloc("sso_string", "operator+=");
            return *this;
        }
        std::memcpy(new_ptr, raw_data_(), size_);
        std::memcpy(new_ptr + size_, sv.data(), sv.size());
        new_ptr[new_size] = '\0';
        free_heap_();
        storage_.heap.ptr = new_ptr;
        storage_.heap.capacity = new_size + 1;
        size_ = new_size;
        is_heap_ = true;
        return *this;
    }

    inline void sso_string_t::clear() noexcept {
        free_heap_();
        size_ = 0;
        storage_.inline_data[0] = '\0';
        is_heap_ = false;
    }

}  // namespace error_system::utils
