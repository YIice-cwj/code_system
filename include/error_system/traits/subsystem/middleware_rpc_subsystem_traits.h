#pragma once

/**
 * @file middleware_rpc_subsystem_traits.h
 * @brief Subsystem middleware_rpc traits specialization
 * @details 自动生成的 subsystem middleware_rpc traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/subsystem/middleware_rpc_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief subsystem::middleware_rpc_subsystem_t 的 traits 特化
     * @details 提供 middleware_rpc_subsystem_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct subsystem_traits<subsystem::middleware_rpc_subsystem_t, void> {

        /** @brief 关联的枚举类型 */
        using subsystem_t = subsystem::middleware_rpc_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::sidecar);
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
                case subsystem_t::grpc: return "grpc";
                case subsystem_t::thrift: return "thrift";
                case subsystem_t::dubbo: return "dubbo";
                case subsystem_t::brpc: return "brpc";
                case subsystem_t::tars: return "tars";
                case subsystem_t::sofa_rpc: return "sofa_rpc";
                case subsystem_t::service_mesh: return "service_mesh";
                case subsystem_t::sidecar: return "sidecar";
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
                case utils::string_utils_t::hash("grpc"): return subsystem_t::grpc;
                case utils::string_utils_t::hash("thrift"): return subsystem_t::thrift;
                case utils::string_utils_t::hash("dubbo"): return subsystem_t::dubbo;
                case utils::string_utils_t::hash("brpc"): return subsystem_t::brpc;
                case utils::string_utils_t::hash("tars"): return subsystem_t::tars;
                case utils::string_utils_t::hash("sofa_rpc"): return subsystem_t::sofa_rpc;
                case utils::string_utils_t::hash("service_mesh"): return subsystem_t::service_mesh;
                case utils::string_utils_t::hash("sidecar"): return subsystem_t::sidecar;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits