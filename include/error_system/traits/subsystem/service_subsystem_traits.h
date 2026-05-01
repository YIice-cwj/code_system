#pragma once
#include "error_system/subsystem/service_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::service_subsystem_t, void> {

        using subsystem_t = subsystem::service_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::collaboration);
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
                case subsystem_t::user:
                    return "user";
                case subsystem_t::auth:
                    return "auth";
                case subsystem_t::order:
                    return "order";
                case subsystem_t::payment:
                    return "payment";
                case subsystem_t::notification:
                    return "notification";
                case subsystem_t::search:
                    return "search";
                case subsystem_t::recommendation:
                    return "recommendation";
                case subsystem_t::cart:
                    return "cart";
                case subsystem_t::inventory:
                    return "inventory";
                case subsystem_t::logistics:
                    return "logistics";
                case subsystem_t::message:
                    return "message";
                case subsystem_t::file:
                    return "file";
                case subsystem_t::content:
                    return "content";
                case subsystem_t::media:
                    return "media";
                case subsystem_t::analytics:
                    return "analytics";
                case subsystem_t::billing:
                    return "billing";
                case subsystem_t::subscription:
                    return "subscription";
                case subsystem_t::social:
                    return "social";
                case subsystem_t::collaboration:
                    return "collaboration";
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
                case utils::string_utils_t::hash("user"):
                    return subsystem_t::user;
                case utils::string_utils_t::hash("auth"):
                    return subsystem_t::auth;
                case utils::string_utils_t::hash("order"):
                    return subsystem_t::order;
                case utils::string_utils_t::hash("payment"):
                    return subsystem_t::payment;
                case utils::string_utils_t::hash("notification"):
                    return subsystem_t::notification;
                case utils::string_utils_t::hash("search"):
                    return subsystem_t::search;
                case utils::string_utils_t::hash("recommendation"):
                    return subsystem_t::recommendation;
                case utils::string_utils_t::hash("cart"):
                    return subsystem_t::cart;
                case utils::string_utils_t::hash("inventory"):
                    return subsystem_t::inventory;
                case utils::string_utils_t::hash("logistics"):
                    return subsystem_t::logistics;
                case utils::string_utils_t::hash("message"):
                    return subsystem_t::message;
                case utils::string_utils_t::hash("file"):
                    return subsystem_t::file;
                case utils::string_utils_t::hash("content"):
                    return subsystem_t::content;
                case utils::string_utils_t::hash("media"):
                    return subsystem_t::media;
                case utils::string_utils_t::hash("analytics"):
                    return subsystem_t::analytics;
                case utils::string_utils_t::hash("billing"):
                    return subsystem_t::billing;
                case utils::string_utils_t::hash("subscription"):
                    return subsystem_t::subscription;
                case utils::string_utils_t::hash("social"):
                    return subsystem_t::social;
                case utils::string_utils_t::hash("collaboration"):
                    return subsystem_t::collaboration;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
