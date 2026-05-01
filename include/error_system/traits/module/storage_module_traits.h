#pragma once

/**
 * @file storage_module_traits.h
 * @brief Module storage traits specialization
 * @details 自动生成的 module storage traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/storage_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::storage_module_t 的 traits 特化
     * @details 提供 storage_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::storage_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::storage_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::garbage_collector);
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
                case module_t::file_manager: return "file_manager";
                case module_t::block_manager: return "block_manager";
                case module_t::object_manager: return "object_manager";
                case module_t::volume_manager: return "volume_manager";
                case module_t::snapshot_manager: return "snapshot_manager";
                case module_t::backup_manager: return "backup_manager";
                case module_t::replication: return "replication";
                case module_t::deduplicator: return "deduplicator";
                case module_t::compressor: return "compressor";
                case module_t::encryptor: return "encryptor";
                case module_t::quota_manager: return "quota_manager";
                case module_t::lifecycle: return "lifecycle";
                case module_t::tiering: return "tiering";
                case module_t::cache_manager: return "cache_manager";
                case module_t::garbage_collector: return "garbage_collector";
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
                case utils::string_utils_t::hash("file_manager"): return module_t::file_manager;
                case utils::string_utils_t::hash("block_manager"): return module_t::block_manager;
                case utils::string_utils_t::hash("object_manager"): return module_t::object_manager;
                case utils::string_utils_t::hash("volume_manager"): return module_t::volume_manager;
                case utils::string_utils_t::hash("snapshot_manager"): return module_t::snapshot_manager;
                case utils::string_utils_t::hash("backup_manager"): return module_t::backup_manager;
                case utils::string_utils_t::hash("replication"): return module_t::replication;
                case utils::string_utils_t::hash("deduplicator"): return module_t::deduplicator;
                case utils::string_utils_t::hash("compressor"): return module_t::compressor;
                case utils::string_utils_t::hash("encryptor"): return module_t::encryptor;
                case utils::string_utils_t::hash("quota_manager"): return module_t::quota_manager;
                case utils::string_utils_t::hash("lifecycle"): return module_t::lifecycle;
                case utils::string_utils_t::hash("tiering"): return module_t::tiering;
                case utils::string_utils_t::hash("cache_manager"): return module_t::cache_manager;
                case utils::string_utils_t::hash("garbage_collector"): return module_t::garbage_collector;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits