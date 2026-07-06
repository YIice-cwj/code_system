#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"

/**
 * @file error_code.h
 * @brief 错误码数据类定义
 * @details 定义 64 位错误码的位域布局、字段解析与访问接口。
 *          采用位移实现，避免位域排布的未定义行为。
 *          位域布局（高→低）：
 *            bit 63     sign      (0=成功，非0=失败)
 *            bit 62-60  reserved  (含 retryable/transient 语义)
 *            bit 59-56  level     (错误等级)
 *            bit 55-48  system    (系统域)
 *            bit 47-32  subsystem (子系统 ID)
 *            bit 31-16  module    (模块 ID)
 *            bit 15-0   number    (错误编号)
 * @author yiice
 * @version 3.0.0
 * @date 2026-05-21
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 错误码类型（64 位无符号整型）
     */
    using code_t = uint64_t;

    /**
     * @brief 模块组 ID 类型（系统域 + 子系统 + 模块）
     */
    using module_group_id_t = uint64_t;

    /**
     * @brief 子系统 ID 强类型包装
     * @details 防止 subsystem/module/number 三个 uint16_t 参数传反。
     */
    struct subsystem_id_t {
        uint16_t value{0};

        explicit constexpr subsystem_id_t(uint16_t v) noexcept;
    };

    /**
     * @brief 模块 ID 强类型包装
     * @details 防止 subsystem/module/number 三个 uint16_t 参数传反。
     */
    struct module_id_t {
        uint16_t value{0};

        explicit constexpr module_id_t(uint16_t v) noexcept;
    };

    /**
     * @brief 错误编号强类型包装
     * @details 防止 subsystem/module/number 三个 uint16_t 参数传反。
     */
    struct error_number_t {
        uint16_t value{0};

        explicit constexpr error_number_t(uint16_t v) noexcept;
    };

    /**
     * @brief 错误码数据类
     * @details 封装 64 位错误码，提供字段解析和访问功能。
     *          基于位移操作实现，避免严格别名规则与位域排布未定义行为。
     */
    class error_code_t {
    public:
        /**
         * @brief Reserved 3 bits 布局
         * @details bit0 = retryable（可重试），bit1 = transient（瞬态），bit2 = reserved。
         *          默认均为 0，与历史行为兼容。
         */
        static constexpr uint32_t SIGN_SHIFT = 63;
        static constexpr uint32_t RESERVED_SHIFT = 60;
        static constexpr uint32_t LEVEL_SHIFT = 56;
        static constexpr uint32_t SYSTEM_SHIFT = 48;
        static constexpr uint32_t SUBSYS_SHIFT = 32;
        static constexpr uint32_t MODULE_SHIFT = 16;
        static constexpr uint32_t NUMBER_SHIFT = 0;

        static constexpr uint64_t SIGN_MASK = 0x1ULL;
        static constexpr uint64_t RESERVED_MASK = 0x7ULL;
        static constexpr uint64_t LEVEL_MASK = 0xFULL;
        static constexpr uint64_t SYSTEM_MASK = 0xFFULL;
        static constexpr uint64_t SUBSYS_MASK = 0xFFFFULL;
        static constexpr uint64_t MODULE_MASK = 0xFFFFULL;
        static constexpr uint64_t NUMBER_MASK = 0xFFFFULL;

        static constexpr uint32_t RETRYABLE_BIT = 0;
        static constexpr uint32_t TRANSIENT_BIT = 1;
        static constexpr uint64_t RETRYABLE_MASK = 0x1ULL;
        static constexpr uint64_t TRANSIENT_MASK = 0x2ULL;

    private:
        code_t code_{0};

    public:
        /**
         * @brief 默认构造为成功码（sign=0）
         * @details 遵循 Unix 退出码约定：0=成功，非0=失败。
         */
        constexpr error_code_t() noexcept;

        constexpr error_code_t(const error_code_t&) noexcept = default;
        constexpr error_code_t(error_code_t&&) noexcept = default;
        constexpr error_code_t& operator=(const error_code_t&) noexcept = default;
        constexpr error_code_t& operator=(error_code_t&&) noexcept = default;
        ~error_code_t() noexcept = default;

        /**
         * @brief 从原始 64 位码值构造
         * @param code 原始 64 位错误码
         */
        constexpr explicit error_code_t(code_t code) noexcept;

        /**
         * @brief 通过位域值构造错误码
         * @details sign 位固定为 1（非0=失败），遵循 Unix 退出码约定。
         * @param level 错误等级 (bits 59-56)
         * @param system 系统域 (bits 55-48)
         * @param subsystem 子系统 ID (bits 47-32)
         * @param module 模块 ID (bits 31-16)
         * @param number 错误编号 (bits 15-0)
         *
         * @example
         * error_code_t code(error_level_t::error, system_domain_t::database,
         *                   subsystem_id_t{1}, module_id_t{2}, error_number_t{0x0010});
         */
        constexpr error_code_t(error_level_t level, domain::system_domain_t system,
                               subsystem_id_t subsystem, module_id_t module, error_number_t number) noexcept;

        /**
         * @brief 创建成功码的工厂方法
         * @details 等价于默认构造函数，语义更清晰。
         * @return 成功码
         */
        static constexpr error_code_t make_success() noexcept;

        /**
         * @brief 获取原始错误码
         * @return 64 位原始错误码
         */
        [[nodiscard]] constexpr code_t get_code() const noexcept;

        /**
         * @brief 获取符号位
         * @return 0=成功，非0=失败
         */
        [[nodiscard]] constexpr uint8_t get_sign() const noexcept;

        /**
         * @brief 判断是否表示失败（sign != 0）
         * @return 失败返回 true，否则 false
         */
        [[nodiscard]] constexpr bool is_error_code() const noexcept;

        /**
         * @brief 判断是否表示成功（sign == 0）
         * @return 成功返回 true，否则 false
         */
        [[nodiscard]] constexpr bool is_success_code() const noexcept;

        /**
         * @brief 获取预留位
         * @return 3 位预留字段值
         */
        [[nodiscard]] constexpr uint8_t get_reserved() const noexcept;

        /**
         * @brief 获取错误等级
         * @details 非法值回退为 fatal，避免下游越界。
         * @return 错误等级枚举
         */
        [[nodiscard]] constexpr error_level_t get_level() const noexcept;

        /**
         * @brief 获取系统域
         * @details 非法值回退为 none，避免下游越界。
         * @return 系统域枚举
         */
        [[nodiscard]] constexpr domain::system_domain_t get_system() const noexcept;

        /**
         * @brief 获取子系统值 (bits 47-32)
         * @return 子系统 ID
         */
        [[nodiscard]] constexpr uint16_t get_subsys() const noexcept;

        /**
         * @brief 获取模块值 (bits 31-16)
         * @return 模块 ID
         */
        [[nodiscard]] constexpr uint16_t get_module() const noexcept;

        /**
         * @brief 获取错误编号 (bits 15-0)
         * @return 错误编号
         */
        [[nodiscard]] constexpr uint16_t get_number() const noexcept;

        /**
         * @brief 获取模块组 ID（系统域 + 子系统 + 模块）
         * @details 高 8 位与低 16 位置零，保留中间 40 位。
         * @return 模块组 ID
         */
        [[nodiscard]] constexpr module_group_id_t get_module_group_id() const noexcept;

        /**
         * @brief 获取清除符号位和预留位后的错误码
         * @details 用于注册和查询时统一忽略这些差异。
         * @return 身份码
         */
        [[nodiscard]] constexpr code_t get_identity_code() const noexcept;

        /**
         * @brief 查询错误是否可重试
         * @details 读取 Reserved.bit0。retryable=1 表示业务可对该错误执行重试逻辑
         *          （如网络抖动、临时限流、leader 切换等）。
         * @return 可重试返回 true，否则 false
         */
        [[nodiscard]] constexpr bool is_retryable() const noexcept;

        /**
         * @brief 查询错误是否为瞬态
         * @details 读取 Reserved.bit1。transient=1 表示该错误可能短时间内自动恢复
         *          （与 retryable 通常同时为 true）。
         * @return 瞬态返回 true，否则 false
         */
        [[nodiscard]] constexpr bool is_transient() const noexcept;

        /**
         * @brief 设置符号位
         * @details 仅接受 0/1，超范围值视为 1（失败），避免污染其他位。
         * @param sign 符号位值
         */
        constexpr void set_sign(uint8_t sign) noexcept;

        /**
         * @brief 设置预留位
         * @details 仅接受 0-7，超范围值视为 0，避免污染其他位。
         * @param reserved 预留位值
         */
        constexpr void set_reserved(uint8_t reserved) noexcept;

        /**
         * @brief 设置错误是否可重试
         * @param retryable 是否可重试
         */
        constexpr void set_retryable(bool retryable) noexcept;

        /**
         * @brief 设置错误是否为瞬态
         * @param transient 是否为瞬态
         */
        constexpr void set_transient(bool transient) noexcept;

        /**
         * @brief 显式转换为 64 位整型
         * @details explicit 防止意外隐式转换（如 if(error_code) 的布尔上下文误用）。
         * @return 64 位原始错误码
         */
        explicit constexpr operator code_t() const noexcept;

        /**
         * @brief 相等比较
         * @param other 另一个错误码
         * @return 相等返回 true，否则 false
         */
        [[nodiscard]] constexpr bool operator==(const error_code_t& other) const noexcept;

        /**
         * @brief 不相等比较
         * @param other 另一个错误码
         * @return 不相等返回 true，否则 false
         */
        [[nodiscard]] constexpr bool operator!=(const error_code_t& other) const noexcept;

        /**
         * @brief 小于比较
         * @param other 另一个错误码
         * @return 小于返回 true，否则 false
         */
        [[nodiscard]] constexpr bool operator<(const error_code_t& other) const noexcept;
    };

    /**
     * @brief 子系统 ID 构造函数定义
     */
    inline constexpr subsystem_id_t::subsystem_id_t(uint16_t v) noexcept : value(v) {}

    /**
     * @brief 模块 ID 构造函数定义
     */
    inline constexpr module_id_t::module_id_t(uint16_t v) noexcept : value(v) {}

    /**
     * @brief 错误编号构造函数定义
     */
    inline constexpr error_number_t::error_number_t(uint16_t v) noexcept : value(v) {}

    /**
     * @brief 默认构造函数定义
     */
    inline constexpr error_code_t::error_code_t() noexcept : code_(0) {}

    /**
     * @brief 从原始码值构造函数定义
     */
    inline constexpr error_code_t::error_code_t(code_t code) noexcept : code_(code) {}

    /**
     * @brief 位域构造函数定义
     */
    inline constexpr error_code_t::error_code_t(error_level_t level, domain::system_domain_t system,
                                                subsystem_id_t subsystem, module_id_t module, error_number_t number) noexcept
        : code_((1ULL << SIGN_SHIFT)
                | (static_cast<code_t>(level) << LEVEL_SHIFT)
                | (static_cast<code_t>(system) << SYSTEM_SHIFT)
                | (static_cast<code_t>(subsystem.value) << SUBSYS_SHIFT)
                | (static_cast<code_t>(module.value) << MODULE_SHIFT)
                | (static_cast<code_t>(number.value) << NUMBER_SHIFT)) {}

    /**
     * @brief 创建成功码
     */
    inline constexpr error_code_t error_code_t::make_success() noexcept { return error_code_t{}; }

    /**
     * @brief 获取原始错误码
     */
    inline constexpr code_t error_code_t::get_code() const noexcept { return code_; }

    /**
     * @brief 获取符号位
     */
    inline constexpr uint8_t error_code_t::get_sign() const noexcept {
        return static_cast<uint8_t>((code_ >> SIGN_SHIFT) & SIGN_MASK);
    }

    /**
     * @brief 判断是否表示失败
     */
    inline constexpr bool error_code_t::is_error_code() const noexcept { return get_sign() != 0; }

    /**
     * @brief 判断是否表示成功
     */
    inline constexpr bool error_code_t::is_success_code() const noexcept { return get_sign() == 0; }

    /**
     * @brief 获取预留位
     */
    inline constexpr uint8_t error_code_t::get_reserved() const noexcept {
        return static_cast<uint8_t>((code_ >> RESERVED_SHIFT) & RESERVED_MASK);
    }

    /**
     * @brief 获取错误等级
     */
    inline constexpr error_level_t error_code_t::get_level() const noexcept {
        return from_int(static_cast<uint8_t>((code_ >> LEVEL_SHIFT) & LEVEL_MASK));
    }

    /**
     * @brief 获取系统域
     */
    inline constexpr domain::system_domain_t error_code_t::get_system() const noexcept {
        return domain::from_int(static_cast<uint8_t>((code_ >> SYSTEM_SHIFT) & SYSTEM_MASK));
    }

    /**
     * @brief 获取子系统值
     */
    inline constexpr uint16_t error_code_t::get_subsys() const noexcept {
        return static_cast<uint16_t>((code_ >> SUBSYS_SHIFT) & SUBSYS_MASK);
    }

    /**
     * @brief 获取模块值
     */
    inline constexpr uint16_t error_code_t::get_module() const noexcept {
        return static_cast<uint16_t>((code_ >> MODULE_SHIFT) & MODULE_MASK);
    }

    /**
     * @brief 获取错误编号
     */
    inline constexpr uint16_t error_code_t::get_number() const noexcept {
        return static_cast<uint16_t>((code_ >> NUMBER_SHIFT) & NUMBER_MASK);
    }

    /**
     * @brief 获取模块组 ID
     */
    inline constexpr module_group_id_t error_code_t::get_module_group_id() const noexcept {
        return code_ & 0x00FFFFFFFFFF0000ULL;
    }

    /**
     * @brief 获取身份码
     */
    inline constexpr code_t error_code_t::get_identity_code() const noexcept {
        return code_ & ~((SIGN_MASK << SIGN_SHIFT) | (RESERVED_MASK << RESERVED_SHIFT));
    }

    /**
     * @brief 查询是否可重试
     */
    inline constexpr bool error_code_t::is_retryable() const noexcept {
        return ((code_ >> RESERVED_SHIFT) & RETRYABLE_MASK) != 0;
    }

    /**
     * @brief 查询是否为瞬态
     */
    inline constexpr bool error_code_t::is_transient() const noexcept {
        return ((code_ >> RESERVED_SHIFT) & TRANSIENT_MASK) != 0;
    }

    /**
     * @brief 设置符号位
     */
    inline constexpr void error_code_t::set_sign(uint8_t sign) noexcept {
        const code_t sign_value = (sign <= 1) ? static_cast<code_t>(sign) : 1ULL;
        code_ = (code_ & ~(SIGN_MASK << SIGN_SHIFT)) | (sign_value << SIGN_SHIFT);
    }

    /**
     * @brief 设置预留位
     */
    inline constexpr void error_code_t::set_reserved(uint8_t reserved) noexcept {
        const code_t reserved_value = (reserved <= 7) ? static_cast<code_t>(reserved) : 0ULL;
        code_ = (code_ & ~(RESERVED_MASK << RESERVED_SHIFT)) | (reserved_value << RESERVED_SHIFT);
    }

    /**
     * @brief 设置是否可重试
     */
    inline constexpr void error_code_t::set_retryable(bool retryable) noexcept {
        const code_t bit = retryable ? RETRYABLE_MASK : 0ULL;
        code_ = (code_ & ~(RETRYABLE_MASK << RESERVED_SHIFT)) | (bit << RESERVED_SHIFT);
    }

    /**
     * @brief 设置是否为瞬态
     */
    inline constexpr void error_code_t::set_transient(bool transient) noexcept {
        const code_t bit = transient ? TRANSIENT_MASK : 0ULL;
        code_ = (code_ & ~(TRANSIENT_MASK << RESERVED_SHIFT)) | (bit << RESERVED_SHIFT);
    }

    /**
     * @brief 转换为整型
     */
    inline constexpr error_code_t::operator code_t() const noexcept { return code_; }

    /**
     * @brief 相等比较
     */
    inline constexpr bool error_code_t::operator==(const error_code_t& other) const noexcept { return code_ == other.code_; }

    /**
     * @brief 不相等比较
     */
    inline constexpr bool error_code_t::operator!=(const error_code_t& other) const noexcept { return code_ != other.code_; }

    /**
     * @brief 小于比较
     */
    inline constexpr bool error_code_t::operator<(const error_code_t& other) const noexcept { return code_ < other.code_; }

    /**
     * @brief 编译期检测错误码数组中是否存在重复
     * @details O(n^2) constexpr 暴力比较，适合小规模（<100）编译期常量数组。
     *          配合 static_assert 使用，在编译期捕获重复错误码定义。
     * @tparam N 数组大小
     */
    template <size_t N>
    [[nodiscard]] constexpr bool all_unique(const std::array<error_code_t, N>& codes) noexcept {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = i + 1; j < N; ++j) {
                if (codes[i] == codes[j]) {
                    return false;
                }
            }
        }
        return true;
    }

}  // namespace error_system::core
