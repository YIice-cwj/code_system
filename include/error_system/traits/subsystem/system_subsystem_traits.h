#pragma once
#include "error_system/subsystem/system_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::system_subsystem_t, void> {

        using subsystem_t = subsystem::system_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::plugin);
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
                case subsystem_t::process:
                    return "process";
                case subsystem_t::thread:
                    return "thread";
                case subsystem_t::memory:
                    return "memory";
                case subsystem_t::signal:
                    return "signal";
                case subsystem_t::timer:
                    return "timer";
                case subsystem_t::ipc:
                    return "ipc";
                case subsystem_t::io:
                    return "io";
                case subsystem_t::filesystem:
                    return "filesystem";
                case subsystem_t::event:
                    return "event";
                case subsystem_t::resource:
                    return "resource";
                case subsystem_t::power:
                    return "power";
                case subsystem_t::log:
                    return "log";
                case subsystem_t::plugin:
                    return "plugin";
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
                case utils::string_utils_t::hash("process"):
                    return subsystem_t::process;
                case utils::string_utils_t::hash("thread"):
                    return subsystem_t::thread;
                case utils::string_utils_t::hash("memory"):
                    return subsystem_t::memory;
                case utils::string_utils_t::hash("signal"):
                    return subsystem_t::signal;
                case utils::string_utils_t::hash("timer"):
                    return subsystem_t::timer;
                case utils::string_utils_t::hash("ipc"):
                    return subsystem_t::ipc;
                case utils::string_utils_t::hash("io"):
                    return subsystem_t::io;
                case utils::string_utils_t::hash("filesystem"):
                    return subsystem_t::filesystem;
                case utils::string_utils_t::hash("event"):
                    return subsystem_t::event;
                case utils::string_utils_t::hash("resource"):
                    return subsystem_t::resource;
                case utils::string_utils_t::hash("power"):
                    return subsystem_t::power;
                case utils::string_utils_t::hash("log"):
                    return subsystem_t::log;
                case utils::string_utils_t::hash("plugin"):
                    return subsystem_t::plugin;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
