#pragma once
#include "error_system/subsystem/middleware_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::middleware_subsystem_t, void> {

        using subsystem_t = subsystem::middleware_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::shardingsphere);
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
                case subsystem_t::rpc:
                    return "rpc";
                case subsystem_t::service_mesh:
                    return "service_mesh";
                case subsystem_t::api_gateway:
                    return "api_gateway";
                case subsystem_t::load_balancer:
                    return "load_balancer";
                case subsystem_t::config_center:
                    return "config_center";
                case subsystem_t::registry:
                    return "registry";
                case subsystem_t::circuit_breaker:
                    return "circuit_breaker";
                case subsystem_t::message_queue:
                    return "message_queue";
                case subsystem_t::cache:
                    return "cache";
                case subsystem_t::search_engine:
                    return "search_engine";
                case subsystem_t::job_scheduler:
                    return "job_scheduler";
                case subsystem_t::data_pipeline:
                    return "data_pipeline";
                case subsystem_t::id_generator:
                    return "id_generator";
                case subsystem_t::lock_service:
                    return "lock_service";
                case subsystem_t::kafka:
                    return "kafka";
                case subsystem_t::rabbitmq:
                    return "rabbitmq";
                case subsystem_t::rocketmq:
                    return "rocketmq";
                case subsystem_t::pulsar:
                    return "pulsar";
                case subsystem_t::nats:
                    return "nats";
                case subsystem_t::memcached:
                    return "memcached";
                case subsystem_t::zookeeper:
                    return "zookeeper";
                case subsystem_t::consul:
                    return "consul";
                case subsystem_t::nginx:
                    return "nginx";
                case subsystem_t::haproxy:
                    return "haproxy";
                case subsystem_t::envoy:
                    return "envoy";
                case subsystem_t::istio:
                    return "istio";
                case subsystem_t::dapr:
                    return "dapr";
                case subsystem_t::seata:
                    return "seata";
                case subsystem_t::shardingsphere:
                    return "shardingsphere";
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
                case utils::string_utils_t::hash("rpc"):
                    return subsystem_t::rpc;
                case utils::string_utils_t::hash("service_mesh"):
                    return subsystem_t::service_mesh;
                case utils::string_utils_t::hash("api_gateway"):
                    return subsystem_t::api_gateway;
                case utils::string_utils_t::hash("load_balancer"):
                    return subsystem_t::load_balancer;
                case utils::string_utils_t::hash("config_center"):
                    return subsystem_t::config_center;
                case utils::string_utils_t::hash("registry"):
                    return subsystem_t::registry;
                case utils::string_utils_t::hash("circuit_breaker"):
                    return subsystem_t::circuit_breaker;
                case utils::string_utils_t::hash("message_queue"):
                    return subsystem_t::message_queue;
                case utils::string_utils_t::hash("cache"):
                    return subsystem_t::cache;
                case utils::string_utils_t::hash("search_engine"):
                    return subsystem_t::search_engine;
                case utils::string_utils_t::hash("job_scheduler"):
                    return subsystem_t::job_scheduler;
                case utils::string_utils_t::hash("data_pipeline"):
                    return subsystem_t::data_pipeline;
                case utils::string_utils_t::hash("id_generator"):
                    return subsystem_t::id_generator;
                case utils::string_utils_t::hash("lock_service"):
                    return subsystem_t::lock_service;
                case utils::string_utils_t::hash("kafka"):
                    return subsystem_t::kafka;
                case utils::string_utils_t::hash("rabbitmq"):
                    return subsystem_t::rabbitmq;
                case utils::string_utils_t::hash("rocketmq"):
                    return subsystem_t::rocketmq;
                case utils::string_utils_t::hash("pulsar"):
                    return subsystem_t::pulsar;
                case utils::string_utils_t::hash("nats"):
                    return subsystem_t::nats;
                case utils::string_utils_t::hash("memcached"):
                    return subsystem_t::memcached;
                case utils::string_utils_t::hash("zookeeper"):
                    return subsystem_t::zookeeper;
                case utils::string_utils_t::hash("consul"):
                    return subsystem_t::consul;
                case utils::string_utils_t::hash("nginx"):
                    return subsystem_t::nginx;
                case utils::string_utils_t::hash("haproxy"):
                    return subsystem_t::haproxy;
                case utils::string_utils_t::hash("envoy"):
                    return subsystem_t::envoy;
                case utils::string_utils_t::hash("istio"):
                    return subsystem_t::istio;
                case utils::string_utils_t::hash("dapr"):
                    return subsystem_t::dapr;
                case utils::string_utils_t::hash("seata"):
                    return subsystem_t::seata;
                case utils::string_utils_t::hash("shardingsphere"):
                    return subsystem_t::shardingsphere;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
