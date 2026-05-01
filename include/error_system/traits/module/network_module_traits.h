#pragma once

/**
 * @file network_module_traits.h
 * @brief Module network traits specialization
 * @details 自动生成的 module network traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/network_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::network_module_t 的 traits 特化
     * @details 提供 network_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::network_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::network_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::service_discovery);
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
                case module_t::socket: return "socket";
                case module_t::tcp_stack: return "tcp_stack";
                case module_t::udp_stack: return "udp_stack";
                case module_t::http_client: return "http_client";
                case module_t::http_server: return "http_server";
                case module_t::websocket: return "websocket";
                case module_t::grpc_client: return "grpc_client";
                case module_t::grpc_server: return "grpc_server";
                case module_t::dns_resolver: return "dns_resolver";
                case module_t::load_balancer: return "load_balancer";
                case module_t::proxy: return "proxy";
                case module_t::gateway: return "gateway";
                case module_t::ssl_context: return "ssl_context";
                case module_t::tls_handshake: return "tls_handshake";
                case module_t::connection_pool: return "connection_pool";
                case module_t::packet_filter: return "packet_filter";
                case module_t::rate_limiter: return "rate_limiter";
                case module_t::circuit_breaker: return "circuit_breaker";
                case module_t::retryer: return "retryer";
                case module_t::health_checker: return "health_checker";
                case module_t::service_discovery: return "service_discovery";
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
                case utils::string_utils_t::hash("socket"): return module_t::socket;
                case utils::string_utils_t::hash("tcp_stack"): return module_t::tcp_stack;
                case utils::string_utils_t::hash("udp_stack"): return module_t::udp_stack;
                case utils::string_utils_t::hash("http_client"): return module_t::http_client;
                case utils::string_utils_t::hash("http_server"): return module_t::http_server;
                case utils::string_utils_t::hash("websocket"): return module_t::websocket;
                case utils::string_utils_t::hash("grpc_client"): return module_t::grpc_client;
                case utils::string_utils_t::hash("grpc_server"): return module_t::grpc_server;
                case utils::string_utils_t::hash("dns_resolver"): return module_t::dns_resolver;
                case utils::string_utils_t::hash("load_balancer"): return module_t::load_balancer;
                case utils::string_utils_t::hash("proxy"): return module_t::proxy;
                case utils::string_utils_t::hash("gateway"): return module_t::gateway;
                case utils::string_utils_t::hash("ssl_context"): return module_t::ssl_context;
                case utils::string_utils_t::hash("tls_handshake"): return module_t::tls_handshake;
                case utils::string_utils_t::hash("connection_pool"): return module_t::connection_pool;
                case utils::string_utils_t::hash("packet_filter"): return module_t::packet_filter;
                case utils::string_utils_t::hash("rate_limiter"): return module_t::rate_limiter;
                case utils::string_utils_t::hash("circuit_breaker"): return module_t::circuit_breaker;
                case utils::string_utils_t::hash("retryer"): return module_t::retryer;
                case utils::string_utils_t::hash("health_checker"): return module_t::health_checker;
                case utils::string_utils_t::hash("service_discovery"): return module_t::service_discovery;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits