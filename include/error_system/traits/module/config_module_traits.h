#pragma once

/**
 * @file config_module_traits.h
 * @brief Module config traits specialization
 * @details 自动生成的 module config traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/config_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::config_module_t 的 traits 特化
     * @details 提供 config_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::config_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::config_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::rollbacker);
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
                case module_t::loader: return "loader";
                case module_t::parser: return "parser";
                case module_t::validator: return "validator";
                case module_t::merger: return "merger";
                case module_t::watcher: return "watcher";
                case module_t::reloader: return "reloader";
                case module_t::encryptor: return "encryptor";
                case module_t::decryptor: return "decryptor";
                case module_t::templater: return "templater";
                case module_t::interpolator: return "interpolator";
                case module_t::schema: return "schema";
                case module_t::migrator: return "migrator";
                case module_t::differ: return "differ";
                case module_t::snapshot: return "snapshot";
                case module_t::rollbacker: return "rollbacker";
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
                case utils::string_utils_t::hash("loader"): return module_t::loader;
                case utils::string_utils_t::hash("parser"): return module_t::parser;
                case utils::string_utils_t::hash("validator"): return module_t::validator;
                case utils::string_utils_t::hash("merger"): return module_t::merger;
                case utils::string_utils_t::hash("watcher"): return module_t::watcher;
                case utils::string_utils_t::hash("reloader"): return module_t::reloader;
                case utils::string_utils_t::hash("encryptor"): return module_t::encryptor;
                case utils::string_utils_t::hash("decryptor"): return module_t::decryptor;
                case utils::string_utils_t::hash("templater"): return module_t::templater;
                case utils::string_utils_t::hash("interpolator"): return module_t::interpolator;
                case utils::string_utils_t::hash("schema"): return module_t::schema;
                case utils::string_utils_t::hash("migrator"): return module_t::migrator;
                case utils::string_utils_t::hash("differ"): return module_t::differ;
                case utils::string_utils_t::hash("snapshot"): return module_t::snapshot;
                case utils::string_utils_t::hash("rollbacker"): return module_t::rollbacker;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits