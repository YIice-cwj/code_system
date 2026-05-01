#pragma once
#include "error_system/module/middleware_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::middleware_module_t, void> {

        using module_t = module::middleware_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::data_sink);
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
                case module_t::rpc_client:
                    return "rpc_client";
                case module_t::rpc_server:
                    return "rpc_server";
                case module_t::service_proxy:
                    return "service_proxy";
                case module_t::service_stub:
                    return "service_stub";
                case module_t::api_gateway:
                    return "api_gateway";
                case module_t::load_balancer:
                    return "load_balancer";
                case module_t::circuit_breaker:
                    return "circuit_breaker";
                case module_t::rate_limiter:
                    return "rate_limiter";
                case module_t::config_client:
                    return "config_client";
                case module_t::registry_client:
                    return "registry_client";
                case module_t::tracing:
                    return "tracing";
                case module_t::metrics:
                    return "metrics";
                case module_t::message_producer:
                    return "message_producer";
                case module_t::message_consumer:
                    return "message_consumer";
                case module_t::message_broker:
                    return "message_broker";
                case module_t::cache_client:
                    return "cache_client";
                case module_t::cache_cluster:
                    return "cache_cluster";
                case module_t::search_indexer:
                    return "search_indexer";
                case module_t::search_query:
                    return "search_query";
                case module_t::job_trigger:
                    return "job_trigger";
                case module_t::job_executor:
                    return "job_executor";
                case module_t::id_generator:
                    return "id_generator";
                case module_t::lock_client:
                    return "lock_client";
                case module_t::data_source:
                    return "data_source";
                case module_t::data_sink:
                    return "data_sink";
                default:
                    return "none";
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
                case utils::string_utils_t::hash("rpc_client"):
                    return module_t::rpc_client;
                case utils::string_utils_t::hash("rpc_server"):
                    return module_t::rpc_server;
                case utils::string_utils_t::hash("service_proxy"):
                    return module_t::service_proxy;
                case utils::string_utils_t::hash("service_stub"):
                    return module_t::service_stub;
                case utils::string_utils_t::hash("api_gateway"):
                    return module_t::api_gateway;
                case utils::string_utils_t::hash("load_balancer"):
                    return module_t::load_balancer;
                case utils::string_utils_t::hash("circuit_breaker"):
                    return module_t::circuit_breaker;
                case utils::string_utils_t::hash("rate_limiter"):
                    return module_t::rate_limiter;
                case utils::string_utils_t::hash("config_client"):
                    return module_t::config_client;
                case utils::string_utils_t::hash("registry_client"):
                    return module_t::registry_client;
                case utils::string_utils_t::hash("tracing"):
                    return module_t::tracing;
                case utils::string_utils_t::hash("metrics"):
                    return module_t::metrics;
                case utils::string_utils_t::hash("message_producer"):
                    return module_t::message_producer;
                case utils::string_utils_t::hash("message_consumer"):
                    return module_t::message_consumer;
                case utils::string_utils_t::hash("message_broker"):
                    return module_t::message_broker;
                case utils::string_utils_t::hash("cache_client"):
                    return module_t::cache_client;
                case utils::string_utils_t::hash("cache_cluster"):
                    return module_t::cache_cluster;
                case utils::string_utils_t::hash("search_indexer"):
                    return module_t::search_indexer;
                case utils::string_utils_t::hash("search_query"):
                    return module_t::search_query;
                case utils::string_utils_t::hash("job_trigger"):
                    return module_t::job_trigger;
                case utils::string_utils_t::hash("job_executor"):
                    return module_t::job_executor;
                case utils::string_utils_t::hash("id_generator"):
                    return module_t::id_generator;
                case utils::string_utils_t::hash("lock_client"):
                    return module_t::lock_client;
                case utils::string_utils_t::hash("data_source"):
                    return module_t::data_source;
                case utils::string_utils_t::hash("data_sink"):
                    return module_t::data_sink;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
