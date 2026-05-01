#pragma once
#include "error_system/module/service_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::service_module_t, void> {

        using module_t = module::service_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::collaboration_room);
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
                case module_t::user_manager:
                    return "user_manager";
                case module_t::auth_manager:
                    return "auth_manager";
                case module_t::session_manager:
                    return "session_manager";
                case module_t::order_manager:
                    return "order_manager";
                case module_t::payment_manager:
                    return "payment_manager";
                case module_t::inventory_manager:
                    return "inventory_manager";
                case module_t::notification_sender:
                    return "notification_sender";
                case module_t::search_engine:
                    return "search_engine";
                case module_t::recommender:
                    return "recommender";
                case module_t::report_generator:
                    return "report_generator";
                case module_t::workflow_engine:
                    return "workflow_engine";
                case module_t::rule_engine:
                    return "rule_engine";
                case module_t::audit_logger:
                    return "audit_logger";
                case module_t::cart_manager:
                    return "cart_manager";
                case module_t::logistics_tracker:
                    return "logistics_tracker";
                case module_t::message_dispatcher:
                    return "message_dispatcher";
                case module_t::file_storage:
                    return "file_storage";
                case module_t::content_parser:
                    return "content_parser";
                case module_t::media_processor:
                    return "media_processor";
                case module_t::analytics_engine:
                    return "analytics_engine";
                case module_t::billing_calculator:
                    return "billing_calculator";
                case module_t::subscription_manager:
                    return "subscription_manager";
                case module_t::social_graph:
                    return "social_graph";
                case module_t::collaboration_room:
                    return "collaboration_room";
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
                case utils::string_utils_t::hash("user_manager"):
                    return module_t::user_manager;
                case utils::string_utils_t::hash("auth_manager"):
                    return module_t::auth_manager;
                case utils::string_utils_t::hash("session_manager"):
                    return module_t::session_manager;
                case utils::string_utils_t::hash("order_manager"):
                    return module_t::order_manager;
                case utils::string_utils_t::hash("payment_manager"):
                    return module_t::payment_manager;
                case utils::string_utils_t::hash("inventory_manager"):
                    return module_t::inventory_manager;
                case utils::string_utils_t::hash("notification_sender"):
                    return module_t::notification_sender;
                case utils::string_utils_t::hash("search_engine"):
                    return module_t::search_engine;
                case utils::string_utils_t::hash("recommender"):
                    return module_t::recommender;
                case utils::string_utils_t::hash("report_generator"):
                    return module_t::report_generator;
                case utils::string_utils_t::hash("workflow_engine"):
                    return module_t::workflow_engine;
                case utils::string_utils_t::hash("rule_engine"):
                    return module_t::rule_engine;
                case utils::string_utils_t::hash("audit_logger"):
                    return module_t::audit_logger;
                case utils::string_utils_t::hash("cart_manager"):
                    return module_t::cart_manager;
                case utils::string_utils_t::hash("logistics_tracker"):
                    return module_t::logistics_tracker;
                case utils::string_utils_t::hash("message_dispatcher"):
                    return module_t::message_dispatcher;
                case utils::string_utils_t::hash("file_storage"):
                    return module_t::file_storage;
                case utils::string_utils_t::hash("content_parser"):
                    return module_t::content_parser;
                case utils::string_utils_t::hash("media_processor"):
                    return module_t::media_processor;
                case utils::string_utils_t::hash("analytics_engine"):
                    return module_t::analytics_engine;
                case utils::string_utils_t::hash("billing_calculator"):
                    return module_t::billing_calculator;
                case utils::string_utils_t::hash("subscription_manager"):
                    return module_t::subscription_manager;
                case utils::string_utils_t::hash("social_graph"):
                    return module_t::social_graph;
                case utils::string_utils_t::hash("collaboration_room"):
                    return module_t::collaboration_room;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
