#pragma once

/**
 * @file ai_module_traits.h
 * @brief Module ai traits specialization
 * @details 自动生成的 module ai traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/ai_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::ai_module_t 的 traits 特化
     * @details 提供 ai_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::ai_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::ai_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::explainability);
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
                case module_t::model_loader: return "model_loader";
                case module_t::model_trainer: return "model_trainer";
                case module_t::inference_engine: return "inference_engine";
                case module_t::tokenizer: return "tokenizer";
                case module_t::embedder: return "embedder";
                case module_t::vector_search: return "vector_search";
                case module_t::prompt_engineer: return "prompt_engineer";
                case module_t::fine_tuner: return "fine_tuner";
                case module_t::data_labeler: return "data_labeler";
                case module_t::feature_extractor: return "feature_extractor";
                case module_t::preprocessor: return "preprocessor";
                case module_t::postprocessor: return "postprocessor";
                case module_t::batch_predictor: return "batch_predictor";
                case module_t::realtime_predictor: return "realtime_predictor";
                case module_t::model_validator: return "model_validator";
                case module_t::drift_detector: return "drift_detector";
                case module_t::explainability: return "explainability";
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
                case utils::string_utils_t::hash("model_loader"): return module_t::model_loader;
                case utils::string_utils_t::hash("model_trainer"): return module_t::model_trainer;
                case utils::string_utils_t::hash("inference_engine"): return module_t::inference_engine;
                case utils::string_utils_t::hash("tokenizer"): return module_t::tokenizer;
                case utils::string_utils_t::hash("embedder"): return module_t::embedder;
                case utils::string_utils_t::hash("vector_search"): return module_t::vector_search;
                case utils::string_utils_t::hash("prompt_engineer"): return module_t::prompt_engineer;
                case utils::string_utils_t::hash("fine_tuner"): return module_t::fine_tuner;
                case utils::string_utils_t::hash("data_labeler"): return module_t::data_labeler;
                case utils::string_utils_t::hash("feature_extractor"): return module_t::feature_extractor;
                case utils::string_utils_t::hash("preprocessor"): return module_t::preprocessor;
                case utils::string_utils_t::hash("postprocessor"): return module_t::postprocessor;
                case utils::string_utils_t::hash("batch_predictor"): return module_t::batch_predictor;
                case utils::string_utils_t::hash("realtime_predictor"): return module_t::realtime_predictor;
                case utils::string_utils_t::hash("model_validator"): return module_t::model_validator;
                case utils::string_utils_t::hash("drift_detector"): return module_t::drift_detector;
                case utils::string_utils_t::hash("explainability"): return module_t::explainability;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits