#pragma once
#include "error_system/module/edge_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::edge_module_t, void> {

        using module_t = module::edge_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::ota_manager);
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
                case module_t::gateway_controller:
                    return "gateway_controller";
                case module_t::node_agent:
                    return "node_agent";
                case module_t::local_runtime:
                    return "local_runtime";
                case module_t::cache_manager:
                    return "cache_manager";
                case module_t::sync_engine:
                    return "sync_engine";
                case module_t::stream_processor:
                    return "stream_processor";
                case module_t::protocol_adapter:
                    return "protocol_adapter";
                case module_t::device_connector:
                    return "device_connector";
                case module_t::rule_engine:
                    return "rule_engine";
                case module_t::function_runner:
                    return "function_runner";
                case module_t::message_router:
                    return "message_router";
                case module_t::data_filter:
                    return "data_filter";
                case module_t::offline_buffer:
                    return "offline_buffer";
                case module_t::ota_manager:
                    return "ota_manager";
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
                case utils::string_utils_t::hash("gateway_controller"):
                    return module_t::gateway_controller;
                case utils::string_utils_t::hash("node_agent"):
                    return module_t::node_agent;
                case utils::string_utils_t::hash("local_runtime"):
                    return module_t::local_runtime;
                case utils::string_utils_t::hash("cache_manager"):
                    return module_t::cache_manager;
                case utils::string_utils_t::hash("sync_engine"):
                    return module_t::sync_engine;
                case utils::string_utils_t::hash("stream_processor"):
                    return module_t::stream_processor;
                case utils::string_utils_t::hash("protocol_adapter"):
                    return module_t::protocol_adapter;
                case utils::string_utils_t::hash("device_connector"):
                    return module_t::device_connector;
                case utils::string_utils_t::hash("rule_engine"):
                    return module_t::rule_engine;
                case utils::string_utils_t::hash("function_runner"):
                    return module_t::function_runner;
                case utils::string_utils_t::hash("message_router"):
                    return module_t::message_router;
                case utils::string_utils_t::hash("data_filter"):
                    return module_t::data_filter;
                case utils::string_utils_t::hash("offline_buffer"):
                    return module_t::offline_buffer;
                case utils::string_utils_t::hash("ota_manager"):
                    return module_t::ota_manager;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
