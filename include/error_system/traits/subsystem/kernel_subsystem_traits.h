#pragma once
#include "error_system/subsystem/kernel_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::kernel_subsystem_t, void> {

        using subsystem_t = subsystem::kernel_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::kprobe);
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
                case subsystem_t::scheduler:
                    return "scheduler";
                case subsystem_t::interrupt:
                    return "interrupt";
                case subsystem_t::driver:
                    return "driver";
                case subsystem_t::vfs:
                    return "vfs";
                case subsystem_t::netstack:
                    return "netstack";
                case subsystem_t::syscall:
                    return "syscall";
                case subsystem_t::module:
                    return "module";
                case subsystem_t::memory:
                    return "memory";
                case subsystem_t::security:
                    return "security";
                case subsystem_t::tracing:
                    return "tracing";
                case subsystem_t::cgroup:
                    return "cgroup";
                case subsystem_t::namespace_:
                    return "namespace_";
                case subsystem_t::ebpf:
                    return "ebpf";
                case subsystem_t::kprobe:
                    return "kprobe";
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
                case utils::string_utils_t::hash("scheduler"):
                    return subsystem_t::scheduler;
                case utils::string_utils_t::hash("interrupt"):
                    return subsystem_t::interrupt;
                case utils::string_utils_t::hash("driver"):
                    return subsystem_t::driver;
                case utils::string_utils_t::hash("vfs"):
                    return subsystem_t::vfs;
                case utils::string_utils_t::hash("netstack"):
                    return subsystem_t::netstack;
                case utils::string_utils_t::hash("syscall"):
                    return subsystem_t::syscall;
                case utils::string_utils_t::hash("module"):
                    return subsystem_t::module;
                case utils::string_utils_t::hash("memory"):
                    return subsystem_t::memory;
                case utils::string_utils_t::hash("security"):
                    return subsystem_t::security;
                case utils::string_utils_t::hash("tracing"):
                    return subsystem_t::tracing;
                case utils::string_utils_t::hash("cgroup"):
                    return subsystem_t::cgroup;
                case utils::string_utils_t::hash("namespace_"):
                    return subsystem_t::namespace_;
                case utils::string_utils_t::hash("ebpf"):
                    return subsystem_t::ebpf;
                case utils::string_utils_t::hash("kprobe"):
                    return subsystem_t::kprobe;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
