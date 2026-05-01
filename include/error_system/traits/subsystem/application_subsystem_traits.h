#pragma once
#include "error_system/subsystem/application_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::application_subsystem_t, void> {

        using subsystem_t = subsystem::application_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::ar_vr);
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
                case subsystem_t::web:
                    return "web";
                case subsystem_t::mobile:
                    return "mobile";
                case subsystem_t::desktop:
                    return "desktop";
                case subsystem_t::cli:
                    return "cli";
                case subsystem_t::batch:
                    return "batch";
                case subsystem_t::workflow:
                    return "workflow";
                case subsystem_t::rule_engine:
                    return "rule_engine";
                case subsystem_t::game:
                    return "game";
                case subsystem_t::embedded:
                    return "embedded";
                case subsystem_t::microservice:
                    return "microservice";
                case subsystem_t::serverless:
                    return "serverless";
                case subsystem_t::ai:
                    return "ai";
                case subsystem_t::ar_vr:
                    return "ar_vr";
                default:
                    return "none";
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
                case utils::string_utils_t::hash("web"):
                    return subsystem_t::web;
                case utils::string_utils_t::hash("mobile"):
                    return subsystem_t::mobile;
                case utils::string_utils_t::hash("desktop"):
                    return subsystem_t::desktop;
                case utils::string_utils_t::hash("cli"):
                    return subsystem_t::cli;
                case utils::string_utils_t::hash("batch"):
                    return subsystem_t::batch;
                case utils::string_utils_t::hash("workflow"):
                    return subsystem_t::workflow;
                case utils::string_utils_t::hash("rule_engine"):
                    return subsystem_t::rule_engine;
                case utils::string_utils_t::hash("game"):
                    return subsystem_t::game;
                case utils::string_utils_t::hash("embedded"):
                    return subsystem_t::embedded;
                case utils::string_utils_t::hash("microservice"):
                    return subsystem_t::microservice;
                case utils::string_utils_t::hash("serverless"):
                    return subsystem_t::serverless;
                case utils::string_utils_t::hash("ai"):
                    return subsystem_t::ai;
                case utils::string_utils_t::hash("ar_vr"):
                    return subsystem_t::ar_vr;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
