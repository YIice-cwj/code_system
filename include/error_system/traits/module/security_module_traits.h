#pragma once

/**
 * @file security_module_traits.h
 * @brief Module security traits specialization
 * @details 自动生成的 module security traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/security_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::security_module_t 的 traits 特化
     * @details 提供 security_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::security_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::security_module_t;
        /** @brief 枚举底层的整数类型 */
        using underlying_t = std::underlying_type_t<module_t>;

        /**
         * @brief 将枚举值转换为底层整数值
         * @param module 枚举实例
         * @return underlying_t 对应的整数值
         */
        static constexpr underlying_t to_int(module_t module) noexcept {
            return static_cast<underlying_t>(module);
        }

        /**
         * @brief 校验整数值是否在枚举定义的有效范围内
         * @param value 待校验的整数值
         * @return bool 如果有效返回 true，否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(module_t::none) && value <= to_int(module_t::access_control);
        }

        /**
         * @brief 从底层整数值转换为枚举值
         * @param value 整数值
         * @return module_t 对应的枚举值；如果值无效，返回 module_t::none
         */
        static constexpr module_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return module_t::none;
            }
            return static_cast<module_t>(value);
        }

        /**
         * @brief 将枚举值转换为对应的字符串名称
         * @param module 枚举实例
         * @return const char* 对应的字符串名称，若无效返回 "none"
         */
        static constexpr const char* to_string(module_t module) noexcept {
            switch (module) {
                case module_t::none: return "none";
                case module_t::authenticator: return "authenticator";
                case module_t::authorizer: return "authorizer";
                case module_t::encryptor: return "encryptor";
                case module_t::decryptor: return "decryptor";
                case module_t::hasher: return "hasher";
                case module_t::signer: return "signer";
                case module_t::verifier: return "verifier";
                case module_t::certificate: return "certificate";
                case module_t::firewall: return "firewall";
                case module_t::waf: return "waf";
                case module_t::intrusion_detector: return "intrusion_detector";
                case module_t::auditor: return "auditor";
                case module_t::token_manager: return "token_manager";
                case module_t::session_manager: return "session_manager";
                case module_t::captcha: return "captcha";
                case module_t::rate_limiter: return "rate_limiter";
                case module_t::access_control: return "access_control";
                default: return "none";
            }
        }

        /**
         * @brief 从字符串名称转换为对应的枚举值
         * @param string 字符串名称
         * @return module_t 对应的枚举值；如果无匹配，返回 module_t::none
         */
        static constexpr module_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"): return module_t::none;
                case utils::string_utils_t::hash("authenticator"): return module_t::authenticator;
                case utils::string_utils_t::hash("authorizer"): return module_t::authorizer;
                case utils::string_utils_t::hash("encryptor"): return module_t::encryptor;
                case utils::string_utils_t::hash("decryptor"): return module_t::decryptor;
                case utils::string_utils_t::hash("hasher"): return module_t::hasher;
                case utils::string_utils_t::hash("signer"): return module_t::signer;
                case utils::string_utils_t::hash("verifier"): return module_t::verifier;
                case utils::string_utils_t::hash("certificate"): return module_t::certificate;
                case utils::string_utils_t::hash("firewall"): return module_t::firewall;
                case utils::string_utils_t::hash("waf"): return module_t::waf;
                case utils::string_utils_t::hash("intrusion_detector"): return module_t::intrusion_detector;
                case utils::string_utils_t::hash("auditor"): return module_t::auditor;
                case utils::string_utils_t::hash("token_manager"): return module_t::token_manager;
                case utils::string_utils_t::hash("session_manager"): return module_t::session_manager;
                case utils::string_utils_t::hash("captcha"): return module_t::captcha;
                case utils::string_utils_t::hash("rate_limiter"): return module_t::rate_limiter;
                case utils::string_utils_t::hash("access_control"): return module_t::access_control;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits