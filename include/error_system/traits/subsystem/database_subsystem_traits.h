#pragma once
#include "error_system/subsystem/database_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::database_subsystem_t, void> {

        using subsystem_t = subsystem::database_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::etcd);
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
                case subsystem_t::mysql:
                    return "mysql";
                case subsystem_t::redis:
                    return "redis";
                case subsystem_t::mongodb:
                    return "mongodb";
                case subsystem_t::postgresql:
                    return "postgresql";
                case subsystem_t::sqlite:
                    return "sqlite";
                case subsystem_t::elasticsearch:
                    return "elasticsearch";
                case subsystem_t::clickhouse:
                    return "clickhouse";
                case subsystem_t::tidb:
                    return "tidb";
                case subsystem_t::oracle:
                    return "oracle";
                case subsystem_t::sqlserver:
                    return "sqlserver";
                case subsystem_t::mariadb:
                    return "mariadb";
                case subsystem_t::neo4j:
                    return "neo4j";
                case subsystem_t::influxdb:
                    return "influxdb";
                case subsystem_t::etcd:
                    return "etcd";
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
                case utils::string_utils_t::hash("mysql"):
                    return subsystem_t::mysql;
                case utils::string_utils_t::hash("redis"):
                    return subsystem_t::redis;
                case utils::string_utils_t::hash("mongodb"):
                    return subsystem_t::mongodb;
                case utils::string_utils_t::hash("postgresql"):
                    return subsystem_t::postgresql;
                case utils::string_utils_t::hash("sqlite"):
                    return subsystem_t::sqlite;
                case utils::string_utils_t::hash("elasticsearch"):
                    return subsystem_t::elasticsearch;
                case utils::string_utils_t::hash("clickhouse"):
                    return subsystem_t::clickhouse;
                case utils::string_utils_t::hash("tidb"):
                    return subsystem_t::tidb;
                case utils::string_utils_t::hash("oracle"):
                    return subsystem_t::oracle;
                case utils::string_utils_t::hash("sqlserver"):
                    return subsystem_t::sqlserver;
                case utils::string_utils_t::hash("mariadb"):
                    return subsystem_t::mariadb;
                case utils::string_utils_t::hash("neo4j"):
                    return subsystem_t::neo4j;
                case utils::string_utils_t::hash("influxdb"):
                    return subsystem_t::influxdb;
                case utils::string_utils_t::hash("etcd"):
                    return subsystem_t::etcd;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
