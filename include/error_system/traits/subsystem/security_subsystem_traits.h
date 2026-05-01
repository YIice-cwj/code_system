#pragma once
#include "error_system/subsystem/security_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::security_subsystem_t, void> {

        using subsystem_t = subsystem::security_subsystem_t;
        using underlying_t = std::underlying_type_t<subsystem_t>;

        /**
         * @brief 将枚举值转换为整数
         * @param subsystem 枚举值
         * @return 对应的整数值
         */
        static constexpr underlying_t to_int(subsystem_t subsystem) noexcept {
            return static_cast<underlying_t>(subsystem);
        }

        /**
         * @brief 校验整数是否为有效的枚举值
         * @param value 要校验的整数
         * @return 如果是有效的枚举值，返回 true；否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::ddos_protection);
        }

        /**
         * @brief 从整数转换为枚举值
         * @param value 要转换的整数
         * @return 对应的枚举值，如果无效则返回 subsystem_t::none
         */
        static constexpr subsystem_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return subsystem_t::none;
            }
            return static_cast<subsystem_t>(value);
        }

        /**
         * @brief 将枚举值转换为字符串
         * @param subsystem 枚举值
         * @return 对应的字符串
         */
        static constexpr const char* to_string(subsystem_t subsystem) noexcept {
            switch (subsystem) {
                case subsystem_t::none:
                    return "none";
                case subsystem_t::auth:
                    return "auth";
                case subsystem_t::authorization:
                    return "authorization";
                case subsystem_t::encryption:
                    return "encryption";
                case subsystem_t::firewall:
                    return "firewall";
                case subsystem_t::intrusion_detection:
                    return "intrusion_detection";
                case subsystem_t::vulnerability_scan:
                    return "vulnerability_scan";
                case subsystem_t::certificate:
                    return "certificate";
                case subsystem_t::key_management:
                    return "key_management";
                case subsystem_t::sso:
                    return "sso";
                case subsystem_t::mfa:
                    return "mfa";
                case subsystem_t::oauth:
                    return "oauth";
                case subsystem_t::ldap:
                    return "ldap";
                case subsystem_t::hsm:
                    return "hsm";
                case subsystem_t::siem:
                    return "siem";
                case subsystem_t::zero_trust:
                    return "zero_trust";
                case subsystem_t::waf:
                    return "waf";
                case subsystem_t::ddos_protection:
                    return "ddos_protection";
                default:
                    return "unknown";
            }
        }

        /**
         * @brief 从字符串转换为枚举值
         * @param string 要转换的字符串
         * @return 对应的枚举值，如果无效则返回 subsystem_t::none
         */
        static constexpr subsystem_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"):
                    return subsystem_t::none;
                case utils::string_utils_t::hash("auth"):
                    return subsystem_t::auth;
                case utils::string_utils_t::hash("authorization"):
                    return subsystem_t::authorization;
                case utils::string_utils_t::hash("encryption"):
                    return subsystem_t::encryption;
                case utils::string_utils_t::hash("firewall"):
                    return subsystem_t::firewall;
                case utils::string_utils_t::hash("intrusion_detection"):
                    return subsystem_t::intrusion_detection;
                case utils::string_utils_t::hash("vulnerability_scan"):
                    return subsystem_t::vulnerability_scan;
                case utils::string_utils_t::hash("certificate"):
                    return subsystem_t::certificate;
                case utils::string_utils_t::hash("key_management"):
                    return subsystem_t::key_management;
                case utils::string_utils_t::hash("sso"):
                    return subsystem_t::sso;
                case utils::string_utils_t::hash("mfa"):
                    return subsystem_t::mfa;
                case utils::string_utils_t::hash("oauth"):
                    return subsystem_t::oauth;
                case utils::string_utils_t::hash("ldap"):
                    return subsystem_t::ldap;
                case utils::string_utils_t::hash("hsm"):
                    return subsystem_t::hsm;
                case utils::string_utils_t::hash("siem"):
                    return subsystem_t::siem;
                case utils::string_utils_t::hash("zero_trust"):
                    return subsystem_t::zero_trust;
                case utils::string_utils_t::hash("waf"):
                    return subsystem_t::waf;
                case utils::string_utils_t::hash("ddos_protection"):
                    return subsystem_t::ddos_protection;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
