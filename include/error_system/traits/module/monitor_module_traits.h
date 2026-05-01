#pragma once

/**
 * @file monitor_module_traits.h
 * @brief Module monitor traits specialization
 * @details 自动生成的 module monitor traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/monitor_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::monitor_module_t 的 traits 特化
     * @details 提供 monitor_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::monitor_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::monitor_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::threshold_manager);
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
                case module_t::metric_collector: return "metric_collector";
                case module_t::metric_aggregator: return "metric_aggregator";
                case module_t::alert_manager: return "alert_manager";
                case module_t::notifier: return "notifier";
                case module_t::dashboard: return "dashboard";
                case module_t::probe: return "probe";
                case module_t::sampler: return "sampler";
                case module_t::tracer: return "tracer";
                case module_t::profiler: return "profiler";
                case module_t::checker: return "checker";
                case module_t::reporter: return "reporter";
                case module_t::anomaly_detector: return "anomaly_detector";
                case module_t::threshold_manager: return "threshold_manager";
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
                case utils::string_utils_t::hash("metric_collector"): return module_t::metric_collector;
                case utils::string_utils_t::hash("metric_aggregator"): return module_t::metric_aggregator;
                case utils::string_utils_t::hash("alert_manager"): return module_t::alert_manager;
                case utils::string_utils_t::hash("notifier"): return module_t::notifier;
                case utils::string_utils_t::hash("dashboard"): return module_t::dashboard;
                case utils::string_utils_t::hash("probe"): return module_t::probe;
                case utils::string_utils_t::hash("sampler"): return module_t::sampler;
                case utils::string_utils_t::hash("tracer"): return module_t::tracer;
                case utils::string_utils_t::hash("profiler"): return module_t::profiler;
                case utils::string_utils_t::hash("checker"): return module_t::checker;
                case utils::string_utils_t::hash("reporter"): return module_t::reporter;
                case utils::string_utils_t::hash("anomaly_detector"): return module_t::anomaly_detector;
                case utils::string_utils_t::hash("threshold_manager"): return module_t::threshold_manager;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits