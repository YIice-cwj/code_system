#pragma once

/**
 * @file log_module_traits.h
 * @brief Module log traits specialization
 * @details 自动生成的 module log traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/log_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::log_module_t 的 traits 特化
     * @details 提供 log_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::log_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::log_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::buffer);
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
                case module_t::appender: return "appender";
                case module_t::formatter: return "formatter";
                case module_t::collector: return "collector";
                case module_t::aggregator: return "aggregator";
                case module_t::parser: return "parser";
                case module_t::filter: return "filter";
                case module_t::rotator: return "rotator";
                case module_t::archiver: return "archiver";
                case module_t::searcher: return "searcher";
                case module_t::analyzer: return "analyzer";
                case module_t::correlator: return "correlator";
                case module_t::shipper: return "shipper";
                case module_t::buffer: return "buffer";
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
                case utils::string_utils_t::hash("appender"): return module_t::appender;
                case utils::string_utils_t::hash("formatter"): return module_t::formatter;
                case utils::string_utils_t::hash("collector"): return module_t::collector;
                case utils::string_utils_t::hash("aggregator"): return module_t::aggregator;
                case utils::string_utils_t::hash("parser"): return module_t::parser;
                case utils::string_utils_t::hash("filter"): return module_t::filter;
                case utils::string_utils_t::hash("rotator"): return module_t::rotator;
                case utils::string_utils_t::hash("archiver"): return module_t::archiver;
                case utils::string_utils_t::hash("searcher"): return module_t::searcher;
                case utils::string_utils_t::hash("analyzer"): return module_t::analyzer;
                case utils::string_utils_t::hash("correlator"): return module_t::correlator;
                case utils::string_utils_t::hash("shipper"): return module_t::shipper;
                case utils::string_utils_t::hash("buffer"): return module_t::buffer;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits