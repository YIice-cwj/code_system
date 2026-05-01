#pragma once

/**
 * @file subsystem_traits.h
 * @brief 子系统类型萃取模板
 * @details 提供子系统类型的通用转换接口（从/向整数、字符串转换及有效性校验）
 * @author yiice
 * @version 1.0.0
 * @date 2026-05-01
 * @copyright Copyright (c) 2026
 */
namespace error_system::traits {

    /**
     * @brief 子系统类型萃取基类模板
     * @details 提供子系统类型的通用转换接口（从/向整数、字符串转换及有效性校验）
     * @tparam T 子系统枚举类型
     * @tparam Enable 模板使能参数
     * @note 此模板必须被具体的子系统枚举类型进行特化
     */
    template <typename T, typename Enable = void>
    struct subsystem_traits {
        // 下面是被特化的模板应当实现的接口约定（注释作为接口文档）：

        // using subsystem_t = T;

        // using underlying_t = std::underlying_type_t<T>;

        // static constexpr subsystem_t from_int(underlying_t value) noexcept;

        // static constexpr subsystem_t from_string(const char* string) noexcept;

        // static constexpr underlying_t to_int(subsystem_t subsystem) noexcept;

        // static constexpr const char* to_string(subsystem_t subsystem) noexcept;

        // static constexpr bool is_valid(underlying_t value) noexcept;
    };

}  // namespace error_system::traits
