#pragma once
#include "error_system/module/kernel_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::kernel_module_t, void> {

        using module_t = module::kernel_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::kprobe);
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
                case module_t::scheduler:
                    return "scheduler";
                case module_t::interrupt:
                    return "interrupt";
                case module_t::driver:
                    return "driver";
                case module_t::vfs:
                    return "vfs";
                case module_t::netstack:
                    return "netstack";
                case module_t::syscall:
                    return "syscall";
                case module_t::module_loader:
                    return "module_loader";
                case module_t::page_table:
                    return "page_table";
                case module_t::dma:
                    return "dma";
                case module_t::buffer:
                    return "buffer";
                case module_t::cache:
                    return "cache";
                case module_t::security:
                    return "security";
                case module_t::tracing:
                    return "tracing";
                case module_t::cgroup:
                    return "cgroup";
                case module_t::namespace_:
                    return "namespace_";
                case module_t::ebpf:
                    return "ebpf";
                case module_t::kprobe:
                    return "kprobe";
                default:
                    return "none";
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
                case utils::string_utils_t::hash("scheduler"):
                    return module_t::scheduler;
                case utils::string_utils_t::hash("interrupt"):
                    return module_t::interrupt;
                case utils::string_utils_t::hash("driver"):
                    return module_t::driver;
                case utils::string_utils_t::hash("vfs"):
                    return module_t::vfs;
                case utils::string_utils_t::hash("netstack"):
                    return module_t::netstack;
                case utils::string_utils_t::hash("syscall"):
                    return module_t::syscall;
                case utils::string_utils_t::hash("module_loader"):
                    return module_t::module_loader;
                case utils::string_utils_t::hash("page_table"):
                    return module_t::page_table;
                case utils::string_utils_t::hash("dma"):
                    return module_t::dma;
                case utils::string_utils_t::hash("buffer"):
                    return module_t::buffer;
                case utils::string_utils_t::hash("cache"):
                    return module_t::cache;
                case utils::string_utils_t::hash("security"):
                    return module_t::security;
                case utils::string_utils_t::hash("tracing"):
                    return module_t::tracing;
                case utils::string_utils_t::hash("cgroup"):
                    return module_t::cgroup;
                case utils::string_utils_t::hash("namespace_"):
                    return module_t::namespace_;
                case utils::string_utils_t::hash("ebpf"):
                    return module_t::ebpf;
                case utils::string_utils_t::hash("kprobe"):
                    return module_t::kprobe;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
