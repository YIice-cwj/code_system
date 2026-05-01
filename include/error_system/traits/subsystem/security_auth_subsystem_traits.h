#pragma once

/**
 * @file security_auth_subsystem_traits.h
 * @brief Subsystem security_auth traits specialization
 * @details 自动生成的 subsystem security_auth traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/subsystem/security_auth_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief subsystem::security_auth_subsystem_t 的 traits 特化
     * @details 提供 security_auth_subsystem_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct subsystem_traits<subsystem::security_auth_subsystem_t, void> {

        /** @brief 关联的枚举类型 */
        using subsystem_t = subsystem::security_auth_subsystem_t;
        /** @brief 枚举底层的整数类型 */
        using underlying_t = std::underlying_type_t<subsystem_t>;

        /**
         * @brief 将枚举值转换为底层整数值
         * @param subsystem 枚举实例
         * @return underlying_t 对应的整数值
         */
        static constexpr underlying_t to_int(subsystem_t subsystem) noexcept {
            return static_cast<underlying_t>(subsystem);
        }

        /**
         * @brief 校验整数值是否在枚举定义的有效范围内
         * @param value 待校验的整数值
         * @return bool 如果有效返回 true，否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::cas);
        }

        /**
         * @brief 从底层整数值转换为枚举值
         * @param value 整数值
         * @return subsystem_t 对应的枚举值；如果值无效，返回 subsystem_t::none
         */
        static constexpr subsystem_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return subsystem_t::none;
            }
            return static_cast<subsystem_t>(value);
        }

        /**
         * @brief 将枚举值转换为对应的字符串名称
         * @param subsystem 枚举实例
         * @return const char* 对应的字符串名称，若无效返回 "none"
         */
        static constexpr const char* to_string(subsystem_t subsystem) noexcept {
            switch (subsystem) {
                case subsystem_t::none: return "none";
                case subsystem_t::oauth2: return "oauth2";
                case subsystem_t::sso: return "sso";
                case subsystem_t::ldap: return "ldap";
                case subsystem_t::saml: return "saml";
                case subsystem_t::jwt: return "jwt";
                case subsystem_t::mfa: return "mfa";
                case subsystem_t::kerberos: return "kerberos";
                case subsystem_t::cas: return "cas";
                default: return "none";
            }
        }

        /**
         * @brief 从字符串名称转换为对应的枚举值
         * @param string 字符串名称
         * @return subsystem_t 对应的枚举值；如果无匹配，返回 subsystem_t::none
         */
        static constexpr subsystem_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"): return subsystem_t::none;
                case utils::string_utils_t::hash("oauth2"): return subsystem_t::oauth2;
                case utils::string_utils_t::hash("sso"): return subsystem_t::sso;
                case utils::string_utils_t::hash("ldap"): return subsystem_t::ldap;
                case utils::string_utils_t::hash("saml"): return subsystem_t::saml;
                case utils::string_utils_t::hash("jwt"): return subsystem_t::jwt;
                case utils::string_utils_t::hash("mfa"): return subsystem_t::mfa;
                case utils::string_utils_t::hash("kerberos"): return subsystem_t::kerberos;
                case utils::string_utils_t::hash("cas"): return subsystem_t::cas;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits