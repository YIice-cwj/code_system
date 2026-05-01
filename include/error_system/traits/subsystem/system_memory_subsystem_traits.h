#pragma once

/**
 * @file system_memory_subsystem_traits.h
 * @brief Subsystem system_memory traits specialization
 * @details 自动生成的 subsystem system_memory traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/subsystem/system_memory_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief subsystem::system_memory_subsystem_t 的 traits 特化
     * @details 提供 system_memory_subsystem_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct subsystem_traits<subsystem::system_memory_subsystem_t, void> {

        /** @brief 关联的枚举类型 */
        using subsystem_t = subsystem::system_memory_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::defragmenter);
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
                case subsystem_t::allocator: return "allocator";
                case subsystem_t::page_manager: return "page_manager";
                case subsystem_t::swap_manager: return "swap_manager";
                case subsystem_t::cache_manager: return "cache_manager";
                case subsystem_t::buffer_manager: return "buffer_manager";
                case subsystem_t::slab_allocator: return "slab_allocator";
                case subsystem_t::vm_manager: return "vm_manager";
                case subsystem_t::oom_handler: return "oom_handler";
                case subsystem_t::defragmenter: return "defragmenter";
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
                case utils::string_utils_t::hash("allocator"): return subsystem_t::allocator;
                case utils::string_utils_t::hash("page_manager"): return subsystem_t::page_manager;
                case utils::string_utils_t::hash("swap_manager"): return subsystem_t::swap_manager;
                case utils::string_utils_t::hash("cache_manager"): return subsystem_t::cache_manager;
                case utils::string_utils_t::hash("buffer_manager"): return subsystem_t::buffer_manager;
                case utils::string_utils_t::hash("slab_allocator"): return subsystem_t::slab_allocator;
                case utils::string_utils_t::hash("vm_manager"): return subsystem_t::vm_manager;
                case utils::string_utils_t::hash("oom_handler"): return subsystem_t::oom_handler;
                case utils::string_utils_t::hash("defragmenter"): return subsystem_t::defragmenter;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits