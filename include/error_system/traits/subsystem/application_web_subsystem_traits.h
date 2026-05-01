#pragma once

/**
 * @file application_web_subsystem_traits.h
 * @brief Subsystem application_web traits specialization
 * @details 自动生成的 subsystem application_web traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/subsystem/application_web_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief subsystem::application_web_subsystem_t 的 traits 特化
     * @details 提供 application_web_subsystem_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct subsystem_traits<subsystem::application_web_subsystem_t, void> {

        /** @brief 关联的枚举类型 */
        using subsystem_t = subsystem::application_web_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::static_server);
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
                case subsystem_t::http_server: return "http_server";
                case subsystem_t::http_client: return "http_client";
                case subsystem_t::websocket: return "websocket";
                case subsystem_t::rest_api: return "rest_api";
                case subsystem_t::graphql: return "graphql";
                case subsystem_t::template_engine: return "template_engine";
                case subsystem_t::session_store: return "session_store";
                case subsystem_t::cookie_manager: return "cookie_manager";
                case subsystem_t::cors_handler: return "cors_handler";
                case subsystem_t::csrf_protector: return "csrf_protector";
                case subsystem_t::static_server: return "static_server";
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
                case utils::string_utils_t::hash("http_server"): return subsystem_t::http_server;
                case utils::string_utils_t::hash("http_client"): return subsystem_t::http_client;
                case utils::string_utils_t::hash("websocket"): return subsystem_t::websocket;
                case utils::string_utils_t::hash("rest_api"): return subsystem_t::rest_api;
                case utils::string_utils_t::hash("graphql"): return subsystem_t::graphql;
                case utils::string_utils_t::hash("template_engine"): return subsystem_t::template_engine;
                case utils::string_utils_t::hash("session_store"): return subsystem_t::session_store;
                case utils::string_utils_t::hash("cookie_manager"): return subsystem_t::cookie_manager;
                case utils::string_utils_t::hash("cors_handler"): return subsystem_t::cors_handler;
                case utils::string_utils_t::hash("csrf_protector"): return subsystem_t::csrf_protector;
                case utils::string_utils_t::hash("static_server"): return subsystem_t::static_server;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits