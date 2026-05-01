#pragma once
#include "error_system/subsystem/ai_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::ai_subsystem_t, void> {

        using subsystem_t = subsystem::ai_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::multimodal);
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
                case subsystem_t::llm:
                    return "llm";
                case subsystem_t::computer_vision:
                    return "computer_vision";
                case subsystem_t::speech_recognition:
                    return "speech_recognition";
                case subsystem_t::nlp:
                    return "nlp";
                case subsystem_t::recommendation:
                    return "recommendation";
                case subsystem_t::anomaly_detection:
                    return "anomaly_detection";
                case subsystem_t::ocr:
                    return "ocr";
                case subsystem_t::face_recognition:
                    return "face_recognition";
                case subsystem_t::automl:
                    return "automl";
                case subsystem_t::mlpipeline:
                    return "mlpipeline";
                case subsystem_t::model_serving:
                    return "model_serving";
                case subsystem_t::feature_store:
                    return "feature_store";
                case subsystem_t::vector_db:
                    return "vector_db";
                case subsystem_t::rag:
                    return "rag";
                case subsystem_t::agent:
                    return "agent";
                case subsystem_t::multimodal:
                    return "multimodal";
                default:
                    return "none";
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
                case utils::string_utils_t::hash("llm"):
                    return subsystem_t::llm;
                case utils::string_utils_t::hash("computer_vision"):
                    return subsystem_t::computer_vision;
                case utils::string_utils_t::hash("speech_recognition"):
                    return subsystem_t::speech_recognition;
                case utils::string_utils_t::hash("nlp"):
                    return subsystem_t::nlp;
                case utils::string_utils_t::hash("recommendation"):
                    return subsystem_t::recommendation;
                case utils::string_utils_t::hash("anomaly_detection"):
                    return subsystem_t::anomaly_detection;
                case utils::string_utils_t::hash("ocr"):
                    return subsystem_t::ocr;
                case utils::string_utils_t::hash("face_recognition"):
                    return subsystem_t::face_recognition;
                case utils::string_utils_t::hash("automl"):
                    return subsystem_t::automl;
                case utils::string_utils_t::hash("mlpipeline"):
                    return subsystem_t::mlpipeline;
                case utils::string_utils_t::hash("model_serving"):
                    return subsystem_t::model_serving;
                case utils::string_utils_t::hash("feature_store"):
                    return subsystem_t::feature_store;
                case utils::string_utils_t::hash("vector_db"):
                    return subsystem_t::vector_db;
                case utils::string_utils_t::hash("rag"):
                    return subsystem_t::rag;
                case utils::string_utils_t::hash("agent"):
                    return subsystem_t::agent;
                case utils::string_utils_t::hash("multimodal"):
                    return subsystem_t::multimodal;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
