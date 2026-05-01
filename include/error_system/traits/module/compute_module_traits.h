#pragma once

/**
 * @file compute_module_traits.h
 * @brief Module compute traits specialization
 * @details 自动生成的 module compute traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/compute_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::compute_module_t 的 traits 特化
     * @details 提供 compute_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::compute_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::compute_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::merger);
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
                case module_t::executor: return "executor";
                case module_t::scheduler: return "scheduler";
                case module_t::worker_pool: return "worker_pool";
                case module_t::thread_pool: return "thread_pool";
                case module_t::coroutine: return "coroutine";
                case module_t::async_awaiter: return "async_awaiter";
                case module_t::pipeline: return "pipeline";
                case module_t::dag_engine: return "dag_engine";
                case module_t::stream_processor: return "stream_processor";
                case module_t::batch_processor: return "batch_processor";
                case module_t::map_reduce: return "map_reduce";
                case module_t::filter_engine: return "filter_engine";
                case module_t::aggregator: return "aggregator";
                case module_t::sampler: return "sampler";
                case module_t::splitter: return "splitter";
                case module_t::merger: return "merger";
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
                case utils::string_utils_t::hash("executor"): return module_t::executor;
                case utils::string_utils_t::hash("scheduler"): return module_t::scheduler;
                case utils::string_utils_t::hash("worker_pool"): return module_t::worker_pool;
                case utils::string_utils_t::hash("thread_pool"): return module_t::thread_pool;
                case utils::string_utils_t::hash("coroutine"): return module_t::coroutine;
                case utils::string_utils_t::hash("async_awaiter"): return module_t::async_awaiter;
                case utils::string_utils_t::hash("pipeline"): return module_t::pipeline;
                case utils::string_utils_t::hash("dag_engine"): return module_t::dag_engine;
                case utils::string_utils_t::hash("stream_processor"): return module_t::stream_processor;
                case utils::string_utils_t::hash("batch_processor"): return module_t::batch_processor;
                case utils::string_utils_t::hash("map_reduce"): return module_t::map_reduce;
                case utils::string_utils_t::hash("filter_engine"): return module_t::filter_engine;
                case utils::string_utils_t::hash("aggregator"): return module_t::aggregator;
                case utils::string_utils_t::hash("sampler"): return module_t::sampler;
                case utils::string_utils_t::hash("splitter"): return module_t::splitter;
                case utils::string_utils_t::hash("merger"): return module_t::merger;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits