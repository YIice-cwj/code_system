#pragma once
#include "error_system/module/security_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::security_module_t, void> {

        using module_t = module::security_module_t;
        using underlying_t = std::underlying_type_t<module_t>;

        /**
         * @brief 将枚举值转换为整数
         * @param module 枚举值
         * @return 对应的整数值
         */
        static constexpr underlying_t to_int(module_t module) noexcept {
            return static_cast<underlying_t>(module);
        }

        /**
         * @brief 校验整数是否为有效的枚举值
         * @param value 要校验的整数
         * @return 如果是有效的枚举值，返回 true；否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(module_t::none) && value <= to_int(module_t::sandbox);
        }

        /**
         * @brief 从整数转换为枚举值
         * @param value 要转换的整数
         * @return 对应的枚举值，如果无效则返回 module_t::none
         */
        static constexpr module_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return module_t::none;
            }
            return static_cast<module_t>(value);
        }

        /**
         * @brief 将枚举值转换为字符串
         * @param module 枚举值
         * @return 对应的字符串
         */
        static constexpr const char* to_string(module_t module) noexcept {
            switch (module) {
                case module_t::none:
                    return "none";
                case module_t::authenticator:
                    return "authenticator";
                case module_t::authorizer:
                    return "authorizer";
                case module_t::encryptor:
                    return "encryptor";
                case module_t::decryptor:
                    return "decryptor";
                case module_t::hash:
                    return "hash";
                case module_t::signature:
                    return "signature";
                case module_t::certificate_manager:
                    return "certificate_manager";
                case module_t::key_store:
                    return "key_store";
                case module_t::token_generator:
                    return "token_generator";
                case module_t::token_validator:
                    return "token_validator";
                case module_t::session_manager:
                    return "session_manager";
                case module_t::password_policy:
                    return "password_policy";
                case module_t::captcha:
                    return "captcha";
                case module_t::rate_limiter:
                    return "rate_limiter";
                case module_t::ip_filter:
                    return "ip_filter";
                case module_t::bot_detector:
                    return "bot_detector";
                case module_t::threat_intel:
                    return "threat_intel";
                case module_t::sandbox:
                    return "sandbox";
                default:
                    return "unknown";
            }
        }

        /**
         * @brief 从字符串转换为枚举值
         * @param string 要转换的字符串
         * @return 对应的枚举值，如果无效则返回 module_t::none
         */
        static constexpr module_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"):
                    return module_t::none;
                case utils::string_utils_t::hash("authenticator"):
                    return module_t::authenticator;
                case utils::string_utils_t::hash("authorizer"):
                    return module_t::authorizer;
                case utils::string_utils_t::hash("encryptor"):
                    return module_t::encryptor;
                case utils::string_utils_t::hash("decryptor"):
                    return module_t::decryptor;
                case utils::string_utils_t::hash("hash"):
                    return module_t::hash;
                case utils::string_utils_t::hash("signature"):
                    return module_t::signature;
                case utils::string_utils_t::hash("certificate_manager"):
                    return module_t::certificate_manager;
                case utils::string_utils_t::hash("key_store"):
                    return module_t::key_store;
                case utils::string_utils_t::hash("token_generator"):
                    return module_t::token_generator;
                case utils::string_utils_t::hash("token_validator"):
                    return module_t::token_validator;
                case utils::string_utils_t::hash("session_manager"):
                    return module_t::session_manager;
                case utils::string_utils_t::hash("password_policy"):
                    return module_t::password_policy;
                case utils::string_utils_t::hash("captcha"):
                    return module_t::captcha;
                case utils::string_utils_t::hash("rate_limiter"):
                    return module_t::rate_limiter;
                case utils::string_utils_t::hash("ip_filter"):
                    return module_t::ip_filter;
                case utils::string_utils_t::hash("bot_detector"):
                    return module_t::bot_detector;
                case utils::string_utils_t::hash("threat_intel"):
                    return module_t::threat_intel;
                case utils::string_utils_t::hash("sandbox"):
                    return module_t::sandbox;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
