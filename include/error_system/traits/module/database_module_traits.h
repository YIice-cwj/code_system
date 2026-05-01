#pragma once

/**
 * @file database_module_traits.h
 * @brief Module database traits specialization
 * @details 自动生成的 module database traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/database_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::database_module_t 的 traits 特化
     * @details 提供 database_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::database_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::database_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::monitor);
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
                case module_t::connector: return "connector";
                case module_t::connection_pool: return "connection_pool";
                case module_t::transaction: return "transaction";
                case module_t::query_executor: return "query_executor";
                case module_t::statement: return "statement";
                case module_t::cursor: return "cursor";
                case module_t::index_manager: return "index_manager";
                case module_t::lock_manager: return "lock_manager";
                case module_t::migration: return "migration";
                case module_t::replicator: return "replicator";
                case module_t::shard_manager: return "shard_manager";
                case module_t::backup_restore: return "backup_restore";
                case module_t::cache_manager: return "cache_manager";
                case module_t::orm_mapper: return "orm_mapper";
                case module_t::schema_manager: return "schema_manager";
                case module_t::trigger_manager: return "trigger_manager";
                case module_t::view_manager: return "view_manager";
                case module_t::procedure: return "procedure";
                case module_t::function_manager: return "function_manager";
                case module_t::partition_manager: return "partition_manager";
                case module_t::vacuum: return "vacuum";
                case module_t::wal_manager: return "wal_manager";
                case module_t::cluster_manager: return "cluster_manager";
                case module_t::monitor: return "monitor";
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
                case utils::string_utils_t::hash("connector"): return module_t::connector;
                case utils::string_utils_t::hash("connection_pool"): return module_t::connection_pool;
                case utils::string_utils_t::hash("transaction"): return module_t::transaction;
                case utils::string_utils_t::hash("query_executor"): return module_t::query_executor;
                case utils::string_utils_t::hash("statement"): return module_t::statement;
                case utils::string_utils_t::hash("cursor"): return module_t::cursor;
                case utils::string_utils_t::hash("index_manager"): return module_t::index_manager;
                case utils::string_utils_t::hash("lock_manager"): return module_t::lock_manager;
                case utils::string_utils_t::hash("migration"): return module_t::migration;
                case utils::string_utils_t::hash("replicator"): return module_t::replicator;
                case utils::string_utils_t::hash("shard_manager"): return module_t::shard_manager;
                case utils::string_utils_t::hash("backup_restore"): return module_t::backup_restore;
                case utils::string_utils_t::hash("cache_manager"): return module_t::cache_manager;
                case utils::string_utils_t::hash("orm_mapper"): return module_t::orm_mapper;
                case utils::string_utils_t::hash("schema_manager"): return module_t::schema_manager;
                case utils::string_utils_t::hash("trigger_manager"): return module_t::trigger_manager;
                case utils::string_utils_t::hash("view_manager"): return module_t::view_manager;
                case utils::string_utils_t::hash("procedure"): return module_t::procedure;
                case utils::string_utils_t::hash("function_manager"): return module_t::function_manager;
                case utils::string_utils_t::hash("partition_manager"): return module_t::partition_manager;
                case utils::string_utils_t::hash("vacuum"): return module_t::vacuum;
                case utils::string_utils_t::hash("wal_manager"): return module_t::wal_manager;
                case utils::string_utils_t::hash("cluster_manager"): return module_t::cluster_manager;
                case utils::string_utils_t::hash("monitor"): return module_t::monitor;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits