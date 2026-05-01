#pragma once
#include "error_system/module/system_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::system_module_t, void> {

        using module_t = module::system_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::plugin);
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
                case module_t::process:
                    return "process";
                case module_t::thread:
                    return "thread";
                case module_t::memory:
                    return "memory";
                case module_t::signal:
                    return "signal";
                case module_t::timer:
                    return "timer";
                case module_t::ipc:
                    return "ipc";
                case module_t::io:
                    return "io";
                case module_t::event:
                    return "event";
                case module_t::semaphore:
                    return "semaphore";
                case module_t::mutex:
                    return "mutex";
                case module_t::rwlock:
                    return "rwlock";
                case module_t::condition:
                    return "condition";
                case module_t::filesystem:
                    return "filesystem";
                case module_t::resource:
                    return "resource";
                case module_t::power:
                    return "power";
                case module_t::log:
                    return "log";
                case module_t::plugin:
                    return "plugin";
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
                case utils::string_utils_t::hash("process"):
                    return module_t::process;
                case utils::string_utils_t::hash("thread"):
                    return module_t::thread;
                case utils::string_utils_t::hash("memory"):
                    return module_t::memory;
                case utils::string_utils_t::hash("signal"):
                    return module_t::signal;
                case utils::string_utils_t::hash("timer"):
                    return module_t::timer;
                case utils::string_utils_t::hash("ipc"):
                    return module_t::ipc;
                case utils::string_utils_t::hash("io"):
                    return module_t::io;
                case utils::string_utils_t::hash("event"):
                    return module_t::event;
                case utils::string_utils_t::hash("semaphore"):
                    return module_t::semaphore;
                case utils::string_utils_t::hash("mutex"):
                    return module_t::mutex;
                case utils::string_utils_t::hash("rwlock"):
                    return module_t::rwlock;
                case utils::string_utils_t::hash("condition"):
                    return module_t::condition;
                case utils::string_utils_t::hash("filesystem"):
                    return module_t::filesystem;
                case utils::string_utils_t::hash("resource"):
                    return module_t::resource;
                case utils::string_utils_t::hash("power"):
                    return module_t::power;
                case utils::string_utils_t::hash("log"):
                    return module_t::log;
                case utils::string_utils_t::hash("plugin"):
                    return module_t::plugin;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
