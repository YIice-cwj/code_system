#pragma once
#include "error_system/subsystem/cloud_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::cloud_subsystem_t, void> {

        using subsystem_t = subsystem::cloud_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::iam);
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
                case subsystem_t::compute:
                    return "compute";
                case subsystem_t::serverless:
                    return "serverless";
                case subsystem_t::container:
                    return "container";
                case subsystem_t::kubernetes:
                    return "kubernetes";
                case subsystem_t::vm:
                    return "vm";
                case subsystem_t::bare_metal:
                    return "bare_metal";
                case subsystem_t::faas:
                    return "faas";
                case subsystem_t::paas:
                    return "paas";
                case subsystem_t::saas:
                    return "saas";
                case subsystem_t::iaas:
                    return "iaas";
                case subsystem_t::auto_scaling:
                    return "auto_scaling";
                case subsystem_t::load_balancing:
                    return "load_balancing";
                case subsystem_t::cdn:
                    return "cdn";
                case subsystem_t::dns:
                    return "dns";
                case subsystem_t::monitoring:
                    return "monitoring";
                case subsystem_t::logging:
                    return "logging";
                case subsystem_t::iam:
                    return "iam";
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
                case utils::string_utils_t::hash("compute"):
                    return subsystem_t::compute;
                case utils::string_utils_t::hash("serverless"):
                    return subsystem_t::serverless;
                case utils::string_utils_t::hash("container"):
                    return subsystem_t::container;
                case utils::string_utils_t::hash("kubernetes"):
                    return subsystem_t::kubernetes;
                case utils::string_utils_t::hash("vm"):
                    return subsystem_t::vm;
                case utils::string_utils_t::hash("bare_metal"):
                    return subsystem_t::bare_metal;
                case utils::string_utils_t::hash("faas"):
                    return subsystem_t::faas;
                case utils::string_utils_t::hash("paas"):
                    return subsystem_t::paas;
                case utils::string_utils_t::hash("saas"):
                    return subsystem_t::saas;
                case utils::string_utils_t::hash("iaas"):
                    return subsystem_t::iaas;
                case utils::string_utils_t::hash("auto_scaling"):
                    return subsystem_t::auto_scaling;
                case utils::string_utils_t::hash("load_balancing"):
                    return subsystem_t::load_balancing;
                case utils::string_utils_t::hash("cdn"):
                    return subsystem_t::cdn;
                case utils::string_utils_t::hash("dns"):
                    return subsystem_t::dns;
                case utils::string_utils_t::hash("monitoring"):
                    return subsystem_t::monitoring;
                case utils::string_utils_t::hash("logging"):
                    return subsystem_t::logging;
                case utils::string_utils_t::hash("iam"):
                    return subsystem_t::iam;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
