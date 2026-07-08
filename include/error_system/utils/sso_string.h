#pragma once
#include <cstddef>
#include <cstring>
#include <new>
#include <string>
#include <string_view>
#include <utility>

#include "error_system/utils/bad_alloc_handler.h"

/**
 * @file sso_string.h
 * @brief SSO 字符串（小字符串优化）
 * @details 固定 48 字节内联缓冲区，短字符串（≤48 字节）零堆分配，
 *          长字符串回退到堆分配。相比 std::string（libc++ 22 字节 SSO）扩大内联容量，
 *          覆盖更多错误消息场景。所有方法 noexcept，内部 try-catch 处理 bad_alloc。
 *
 *          布局（64 位架构，64 字节对齐）：
 *          - size_ (8B)：当前长度
 *          - is_heap_ (1B) + padding (7B)：模式标志
 *          - storage_ (union, 48B)：内联缓冲区 or {ptr, capacity}
 *          总计 64 字节。
 *
 *          内存释放使用 ::operator delete，避免与 placement new 配对错误。
 * @author yiice
 * @version 1.0.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief SSO 字符串
     * @details 48 字节内联缓冲区，短字符串零堆分配。替代 runtime_block_t 中的
     *          std::string message 字段，消除中等长度错误消息的堆分配开销。
     *          提供 std::string_view 隐式转换，兼容只读场景。
     */
    class sso_string_t {
    public:
        static constexpr size_t SSO_CAPACITY = 48;

        sso_string_t() noexcept = default;

        /**
         * @brief 从 const char* 构造
         * @param s C 字符串，可为 nullptr（构造空字符串）
         */
        explicit sso_string_t(const char* s) noexcept;

        /**
         * @brief 从 string_view 构造
         * @param sv 字符串视图
         */
        sso_string_t(std::string_view sv) noexcept;

        /**
         * @brief 从 std::string 构造
         * @param s 标准字符串
         */
        sso_string_t(const std::string& s) noexcept;

        /**
         * @brief 从 std::string 右值移动构造（可能复用堆内存）
         * @param s 标准字符串右值
         */
        sso_string_t(std::string&& s) noexcept;

        ~sso_string_t() noexcept { free_heap_(); }

        sso_string_t(const sso_string_t& other) noexcept;
        sso_string_t& operator=(const sso_string_t& other) noexcept;
        sso_string_t(sso_string_t&& other) noexcept;
        sso_string_t& operator=(sso_string_t&& other) noexcept;

        sso_string_t& operator=(const char* s) noexcept;
        sso_string_t& operator=(std::string_view sv) noexcept;
        sso_string_t& operator=(const std::string& s) noexcept;
        sso_string_t& operator=(std::string&& s) noexcept;

        /**
         * @brief 获取 C 字符串指针
         * @return 以 null 结尾的 C 字符串
         */
        [[nodiscard]] const char* c_str() const noexcept;

        /**
         * @brief 获取数据指针
         * @return 字符串数据指针（可能非 null 结尾）
         */
        [[nodiscard]] const char* data() const noexcept;

        /**
         * @brief 获取字符串长度
         * @return 长度（字节数）
         */
        [[nodiscard]] size_t size() const noexcept { return size_; }

        /**
         * @brief 是否为空
         * @return 空返回 true
         */
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

        /**
         * @brief 隐式转换为 string_view
         * @return 字符串视图
         */
        [[nodiscard]] operator std::string_view() const noexcept;

        /**
         * @brief 转换为 std::string
         * @return 新构造的 std::string
         */
        [[nodiscard]] std::string to_std_string() const noexcept;

        /**
         * @brief 追加 string_view
         * @param sv 待追加的字符串视图
         * @return 自身引用
         */
        sso_string_t& operator+=(std::string_view sv) noexcept;

        /**
         * @brief 清空字符串（不释放堆内存）
         */
        void clear() noexcept;

    private:
        size_t size_{0};
        bool is_heap_{false};
        union storage_t {
            char inline_data[SSO_CAPACITY + 1];
            struct {
                char* ptr;
                size_t capacity;
            } heap;
            storage_t() noexcept { std::memset(this, 0, sizeof(storage_t)); }
        } storage_;

        void set_inline_(const char* s, size_t len) noexcept;
        void set_heap_(const char* s, size_t len) noexcept;
        void free_heap_() noexcept;
        [[nodiscard]] const char* raw_data_() const noexcept;
    };

    /**
     * @brief SSO 字符串相等比较
     */
    [[nodiscard]] inline bool operator==(const sso_string_t& lhs, std::string_view rhs) noexcept {
        return std::string_view(lhs) == rhs;
    }

    [[nodiscard]] inline bool operator==(std::string_view lhs, const sso_string_t& rhs) noexcept {
        return lhs == std::string_view(rhs);
    }

    [[nodiscard]] inline bool operator==(const sso_string_t& lhs, const sso_string_t& rhs) noexcept {
        return std::string_view(lhs) == std::string_view(rhs);
    }

}  // namespace error_system::utils

#include "error_system/utils/details/sso_string.inl"
