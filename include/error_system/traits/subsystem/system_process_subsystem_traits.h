#pragma once

/**
 * @file system_process_subsystem_traits.h
 * @brief Subsystem system_process traits specialization
 * @details 自动生成的 subsystem system_process traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/subsystem/system_process_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief subsystem::system_process_subsystem_t 的 traits 特化
     * @details 提供 system_process_subsystem_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct subsystem_traits<subsystem::system_process_subsystem_t, void> {

        /** @brief 关联的枚举类型 */
        using subsystem_t = subsystem::system_process_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::daemon_manager);
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
                case subsystem_t::scheduler: return "scheduler";
                case subsystem_t::fork_manager: return "fork_manager";
                case subsystem_t::signal_handler: return "signal_handler";
                case subsystem_t::zombie_reaper: return "zombie_reaper";
                case subsystem_t::priority_manager: return "priority_manager";
                case subsystem_t::cgroup_manager: return "cgroup_manager";
                case subsystem_t::namespace_manager: return "namespace_manager";
                case subsystem_t::ipc_manager: return "ipc_manager";
                case subsystem_t::daemon_manager: return "daemon_manager";
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
                case utils::string_utils_t::hash("scheduler"): return subsystem_t::scheduler;
                case utils::string_utils_t::hash("fork_manager"): return subsystem_t::fork_manager;
                case utils::string_utils_t::hash("signal_handler"): return subsystem_t::signal_handler;
                case utils::string_utils_t::hash("zombie_reaper"): return subsystem_t::zombie_reaper;
                case utils::string_utils_t::hash("priority_manager"): return subsystem_t::priority_manager;
                case utils::string_utils_t::hash("cgroup_manager"): return subsystem_t::cgroup_manager;
                case utils::string_utils_t::hash("namespace_manager"): return subsystem_t::namespace_manager;
                case utils::string_utils_t::hash("ipc_manager"): return subsystem_t::ipc_manager;
                case utils::string_utils_t::hash("daemon_manager"): return subsystem_t::daemon_manager;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits