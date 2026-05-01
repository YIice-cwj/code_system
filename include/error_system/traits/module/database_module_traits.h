#pragma once
#include "error_system/module/database_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::database_module_t, void> {

        using module_t = module::database_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::monitor);
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
                case module_t::connection:
                    return "connection";
                case module_t::connection_pool:
                    return "connection_pool";
                case module_t::transaction:
                    return "transaction";
                case module_t::query:
                    return "query";
                case module_t::statement:
                    return "statement";
                case module_t::cursor:
                    return "cursor";
                case module_t::index:
                    return "index";
                case module_t::lock:
                    return "lock";
                case module_t::migration:
                    return "migration";
                case module_t::replication:
                    return "replication";
                case module_t::sharding:
                    return "sharding";
                case module_t::backup:
                    return "backup";
                case module_t::cache:
                    return "cache";
                case module_t::orm:
                    return "orm";
                case module_t::schema:
                    return "schema";
                case module_t::trigger:
                    return "trigger";
                case module_t::view:
                    return "view";
                case module_t::procedure:
                    return "procedure";
                case module_t::function_:
                    return "function_";
                case module_t::partition:
                    return "partition";
                case module_t::vacuum:
                    return "vacuum";
                case module_t::wal:
                    return "wal";
                case module_t::cluster:
                    return "cluster";
                case module_t::monitor:
                    return "monitor";
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
                case utils::string_utils_t::hash("connection"):
                    return module_t::connection;
                case utils::string_utils_t::hash("connection_pool"):
                    return module_t::connection_pool;
                case utils::string_utils_t::hash("transaction"):
                    return module_t::transaction;
                case utils::string_utils_t::hash("query"):
                    return module_t::query;
                case utils::string_utils_t::hash("statement"):
                    return module_t::statement;
                case utils::string_utils_t::hash("cursor"):
                    return module_t::cursor;
                case utils::string_utils_t::hash("index"):
                    return module_t::index;
                case utils::string_utils_t::hash("lock"):
                    return module_t::lock;
                case utils::string_utils_t::hash("migration"):
                    return module_t::migration;
                case utils::string_utils_t::hash("replication"):
                    return module_t::replication;
                case utils::string_utils_t::hash("sharding"):
                    return module_t::sharding;
                case utils::string_utils_t::hash("backup"):
                    return module_t::backup;
                case utils::string_utils_t::hash("cache"):
                    return module_t::cache;
                case utils::string_utils_t::hash("orm"):
                    return module_t::orm;
                case utils::string_utils_t::hash("schema"):
                    return module_t::schema;
                case utils::string_utils_t::hash("trigger"):
                    return module_t::trigger;
                case utils::string_utils_t::hash("view"):
                    return module_t::view;
                case utils::string_utils_t::hash("procedure"):
                    return module_t::procedure;
                case utils::string_utils_t::hash("function_"):
                    return module_t::function_;
                case utils::string_utils_t::hash("partition"):
                    return module_t::partition;
                case utils::string_utils_t::hash("vacuum"):
                    return module_t::vacuum;
                case utils::string_utils_t::hash("wal"):
                    return module_t::wal;
                case utils::string_utils_t::hash("cluster"):
                    return module_t::cluster;
                case utils::string_utils_t::hash("monitor"):
                    return module_t::monitor;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
