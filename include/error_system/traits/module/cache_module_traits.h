#pragma once

/**
 * @file cache_module_traits.h
 * @brief Module cache traits specialization
 * @details 自动生成的 module cache traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/cache_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::cache_module_t 的 traits 特化
     * @details 提供 cache_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::cache_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::cache_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::partitioner);
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
                case module_t::local_cache: return "local_cache";
                case module_t::distributed_cache: return "distributed_cache";
                case module_t::memory_cache: return "memory_cache";
                case module_t::disk_cache: return "disk_cache";
                case module_t::lru_manager: return "lru_manager";
                case module_t::lfu_manager: return "lfu_manager";
                case module_t::ttl_manager: return "ttl_manager";
                case module_t::eviction: return "eviction";
                case module_t::prefetcher: return "prefetcher";
                case module_t::warmer: return "warmer";
                case module_t::invalidator: return "invalidator";
                case module_t::synchronizer: return "synchronizer";
                case module_t::serializer: return "serializer";
                case module_t::compressor: return "compressor";
                case module_t::partitioner: return "partitioner";
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
                case utils::string_utils_t::hash("local_cache"): return module_t::local_cache;
                case utils::string_utils_t::hash("distributed_cache"): return module_t::distributed_cache;
                case utils::string_utils_t::hash("memory_cache"): return module_t::memory_cache;
                case utils::string_utils_t::hash("disk_cache"): return module_t::disk_cache;
                case utils::string_utils_t::hash("lru_manager"): return module_t::lru_manager;
                case utils::string_utils_t::hash("lfu_manager"): return module_t::lfu_manager;
                case utils::string_utils_t::hash("ttl_manager"): return module_t::ttl_manager;
                case utils::string_utils_t::hash("eviction"): return module_t::eviction;
                case utils::string_utils_t::hash("prefetcher"): return module_t::prefetcher;
                case utils::string_utils_t::hash("warmer"): return module_t::warmer;
                case utils::string_utils_t::hash("invalidator"): return module_t::invalidator;
                case utils::string_utils_t::hash("synchronizer"): return module_t::synchronizer;
                case utils::string_utils_t::hash("serializer"): return module_t::serializer;
                case utils::string_utils_t::hash("compressor"): return module_t::compressor;
                case utils::string_utils_t::hash("partitioner"): return module_t::partitioner;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits