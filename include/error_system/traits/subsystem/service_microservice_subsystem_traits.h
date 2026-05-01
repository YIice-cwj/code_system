#pragma once

/**
 * @file service_microservice_subsystem_traits.h
 * @brief Subsystem service_microservice traits specialization
 * @details 自动生成的 subsystem service_microservice traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/subsystem/service_microservice_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief subsystem::service_microservice_subsystem_t 的 traits 特化
     * @details 提供 service_microservice_subsystem_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct subsystem_traits<subsystem::service_microservice_subsystem_t, void> {

        /** @brief 关联的枚举类型 */
        using subsystem_t = subsystem::service_microservice_subsystem_t;
        /** @brief 枚举底层的整数类型 */
        using underlying_t = std::underlying_type_t<subsystem_t>;

        /**
         * @brief 将枚举值转换为底层整数值
         * @param subsystem 枚举实例
         * @return underlying_t 对应的整数值
         */
        static constexpr underlying_t to_int(subsystem_t subsystem) noexcept {
            return static_cast<underlying_t>(subsystem);
        }

        /**
         * @brief 校验整数值是否在枚举定义的有效范围内
         * @param value 待校验的整数值
         * @return bool 如果有效返回 true，否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::health_check);
        }

        /**
         * @brief 从底层整数值转换为枚举值
         * @param value 整数值
         * @return subsystem_t 对应的枚举值；如果值无效，返回 subsystem_t::none
         */
        static constexpr subsystem_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return subsystem_t::none;
            }
            return static_cast<subsystem_t>(value);
        }

        /**
         * @brief 将枚举值转换为对应的字符串名称
         * @param subsystem 枚举实例
         * @return const char* 对应的字符串名称，若无效返回 "none"
         */
        static constexpr const char* to_string(subsystem_t subsystem) noexcept {
            switch (subsystem) {
                case subsystem_t::none: return "none";
                case subsystem_t::gateway: return "gateway";
                case subsystem_t::registry: return "registry";
                case subsystem_t::discovery: return "discovery";
                case subsystem_t::config_center: return "config_center";
                case subsystem_t::breaker: return "breaker";
                case subsystem_t::limiter: return "limiter";
                case subsystem_t::balancer: return "balancer";
                case subsystem_t::tracing: return "tracing";
                case subsystem_t::metrics: return "metrics";
                case subsystem_t::health_check: return "health_check";
                default: return "none";
            }
        }

        /**
         * @brief 从字符串名称转换为对应的枚举值
         * @param string 字符串名称
         * @return subsystem_t 对应的枚举值；如果无匹配，返回 subsystem_t::none
         */
        static constexpr subsystem_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"): return subsystem_t::none;
                case utils::string_utils_t::hash("gateway"): return subsystem_t::gateway;
                case utils::string_utils_t::hash("registry"): return subsystem_t::registry;
                case utils::string_utils_t::hash("discovery"): return subsystem_t::discovery;
                case utils::string_utils_t::hash("config_center"): return subsystem_t::config_center;
                case utils::string_utils_t::hash("breaker"): return subsystem_t::breaker;
                case utils::string_utils_t::hash("limiter"): return subsystem_t::limiter;
                case utils::string_utils_t::hash("balancer"): return subsystem_t::balancer;
                case utils::string_utils_t::hash("tracing"): return subsystem_t::tracing;
                case utils::string_utils_t::hash("metrics"): return subsystem_t::metrics;
                case utils::string_utils_t::hash("health_check"): return subsystem_t::health_check;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits