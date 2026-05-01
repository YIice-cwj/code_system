#pragma once
#include "error_system/subsystem/edge_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::edge_subsystem_t, void> {

        using subsystem_t = subsystem::edge_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::edge_ml);
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
                case subsystem_t::gateway:
                    return "gateway";
                case subsystem_t::node:
                    return "node";
                case subsystem_t::computing:
                    return "computing";
                case subsystem_t::storage:
                    return "storage";
                case subsystem_t::ai_inference:
                    return "ai_inference";
                case subsystem_t::video_analytics:
                    return "video_analytics";
                case subsystem_t::data_sync:
                    return "data_sync";
                case subsystem_t::device_management:
                    return "device_management";
                case subsystem_t::fog:
                    return "fog";
                case subsystem_t::mec:
                    return "mec";
                case subsystem_t::low_latency:
                    return "low_latency";
                case subsystem_t::offline:
                    return "offline";
                case subsystem_t::edge_ml:
                    return "edge_ml";
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
                case utils::string_utils_t::hash("gateway"):
                    return subsystem_t::gateway;
                case utils::string_utils_t::hash("node"):
                    return subsystem_t::node;
                case utils::string_utils_t::hash("computing"):
                    return subsystem_t::computing;
                case utils::string_utils_t::hash("storage"):
                    return subsystem_t::storage;
                case utils::string_utils_t::hash("ai_inference"):
                    return subsystem_t::ai_inference;
                case utils::string_utils_t::hash("video_analytics"):
                    return subsystem_t::video_analytics;
                case utils::string_utils_t::hash("data_sync"):
                    return subsystem_t::data_sync;
                case utils::string_utils_t::hash("device_management"):
                    return subsystem_t::device_management;
                case utils::string_utils_t::hash("fog"):
                    return subsystem_t::fog;
                case utils::string_utils_t::hash("mec"):
                    return subsystem_t::mec;
                case utils::string_utils_t::hash("low_latency"):
                    return subsystem_t::low_latency;
                case utils::string_utils_t::hash("offline"):
                    return subsystem_t::offline;
                case utils::string_utils_t::hash("edge_ml"):
                    return subsystem_t::edge_ml;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
