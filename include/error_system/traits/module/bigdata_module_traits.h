#pragma once
#include "error_system/module/bigdata_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::bigdata_module_t, void> {

        using module_t = module::bigdata_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::catalog_manager);
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
                case module_t::data_collector:
                    return "data_collector";
                case module_t::extractor:
                    return "extractor";
                case module_t::transformer:
                    return "transformer";
                case module_t::loader:
                    return "loader";
                case module_t::stream_reader:
                    return "stream_reader";
                case module_t::stream_writer:
                    return "stream_writer";
                case module_t::batch_scheduler:
                    return "batch_scheduler";
                case module_t::query_engine:
                    return "query_engine";
                case module_t::cube_builder:
                    return "cube_builder";
                case module_t::model_trainer:
                    return "model_trainer";
                case module_t::graph_loader:
                    return "graph_loader";
                case module_t::window_operator:
                    return "window_operator";
                case module_t::checkpoint_manager:
                    return "checkpoint_manager";
                case module_t::lineage_tracker:
                    return "lineage_tracker";
                case module_t::quality_checker:
                    return "quality_checker";
                case module_t::catalog_manager:
                    return "catalog_manager";
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
                case utils::string_utils_t::hash("data_collector"):
                    return module_t::data_collector;
                case utils::string_utils_t::hash("extractor"):
                    return module_t::extractor;
                case utils::string_utils_t::hash("transformer"):
                    return module_t::transformer;
                case utils::string_utils_t::hash("loader"):
                    return module_t::loader;
                case utils::string_utils_t::hash("stream_reader"):
                    return module_t::stream_reader;
                case utils::string_utils_t::hash("stream_writer"):
                    return module_t::stream_writer;
                case utils::string_utils_t::hash("batch_scheduler"):
                    return module_t::batch_scheduler;
                case utils::string_utils_t::hash("query_engine"):
                    return module_t::query_engine;
                case utils::string_utils_t::hash("cube_builder"):
                    return module_t::cube_builder;
                case utils::string_utils_t::hash("model_trainer"):
                    return module_t::model_trainer;
                case utils::string_utils_t::hash("graph_loader"):
                    return module_t::graph_loader;
                case utils::string_utils_t::hash("window_operator"):
                    return module_t::window_operator;
                case utils::string_utils_t::hash("checkpoint_manager"):
                    return module_t::checkpoint_manager;
                case utils::string_utils_t::hash("lineage_tracker"):
                    return module_t::lineage_tracker;
                case utils::string_utils_t::hash("quality_checker"):
                    return module_t::quality_checker;
                case utils::string_utils_t::hash("catalog_manager"):
                    return module_t::catalog_manager;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
