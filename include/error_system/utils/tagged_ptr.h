#pragma once
#include <cstdint>

/**
 * @file tagged_ptr.h
 * @brief 标记指针工具
 * @details 将 48 位指针与 16 位版本号打包到 64 位原子中，用于无锁数据结构
 *          解决 ABA 问题。每次 CAS 成功后递增版本号，即使指针被分配器复用，
 *          版本号不同也会导致 CAS 失败。仅支持 64 位平台（指针 8 字节，
 *          地址空间 ≤ 48 位）。
 * @author yiice
 * @version 1.0.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief 标记指针
     * @details 将指针与版本号打包为单个 64 位值，支持原子 CAS 操作。
     *          低 48 位存储指针（用 PTR_MASK 屏蔽高位，TBI 安全），
     *          高 16 位存储版本号。所有方法为 noexcept，仅使用整数运算，
     *          可安全用于原子操作。
     * @tparam T 指针指向的对象类型
     * @note 仅支持 64 位平台。版本号溢出回绕（uint16_t）理论上需 65536 次
     *       CAS 恰好复用同一指针才会失效，实际场景几乎不可能发生。
     */
    template <typename T>
    class tagged_ptr_t {
    public:
        /**
         * @brief 打包后的 64 位值类型
         */
        using packed_t = uint64_t;

        /**
         * @brief 版本号类型
         */
        using tag_t = uint16_t;

    private:
        static constexpr uint64_t PTR_MASK = 0x0000FFFFFFFFFFFFULL;
        static constexpr uint64_t TAG_SHIFT = 48;

        packed_t packed_{0};

    public:
        tagged_ptr_t() noexcept = default;

        /**
         * @brief 从指针与版本号构造
         * @param ptr 节点指针（可为 nullptr）
         * @param tag 版本号
         */
        tagged_ptr_t(T* ptr, tag_t tag) noexcept : packed_(pack(ptr, tag)) {}

        /**
         * @brief 从打包值构造
         * @param packed 已打包的 64 位值
         */
        explicit tagged_ptr_t(packed_t packed) noexcept : packed_(packed) {}

        /**
         * @brief 将指针与版本号打包为 64 位值
         * @param ptr 节点指针
         * @param tag 版本号
         * @return 打包后的值，低 48 位为指针，高 16 位为版本号
         */
        static packed_t pack(T* ptr, tag_t tag) noexcept {
            return (reinterpret_cast<uint64_t>(ptr) & PTR_MASK)
                 | (static_cast<uint64_t>(tag) << TAG_SHIFT);
        }

        /**
         * @brief 从打包值提取指针
         * @param packed 打包后的值
         * @return 节点指针
         */
        static T* get_ptr(packed_t packed) noexcept {
            return reinterpret_cast<T*>(packed & PTR_MASK);
        }

        /**
         * @brief 从打包值提取版本号
         * @param packed 打包后的值
         * @return 版本号
         */
        static tag_t get_tag(packed_t packed) noexcept {
            return static_cast<tag_t>(packed >> TAG_SHIFT);
        }

        /**
         * @brief 获取打包值
         * @return 当前打包的 64 位值
         */
        [[nodiscard]] packed_t raw() const noexcept { return packed_; }

        /**
         * @brief 从打包值设置
         * @param packed 打包后的值
         */
        void set_raw(packed_t packed) noexcept { packed_ = packed; }

        /**
         * @brief 获取指针
         * @return 当前指针
         */
        [[nodiscard]] T* ptr() const noexcept { return get_ptr(packed_); }

        /**
         * @brief 获取版本号
         * @return 当前版本号
         */
        [[nodiscard]] tag_t tag() const noexcept { return get_tag(packed_); }

        /**
         * @brief 返回递增版本号后的新打包值
         * @details 用于 CAS 成功后生成下一个标记值，版本号自动回绕。
         * @param ptr 新指针
         * @return 新打包值，版本号 = 当前版本号 + 1
         */
        [[nodiscard]] packed_t next_with(T* ptr) const noexcept {
            return pack(ptr, static_cast<tag_t>(tag() + 1));
        }
    };

}  // namespace error_system::utils

#include "error_system/utils/details/tagged_ptr.inl"
