#pragma once
#include "error_system/subsystem/bigdata_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::bigdata_subsystem_t, void> {

        using subsystem_t = subsystem::bigdata_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::data_governance);
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
                case subsystem_t::ingestion:
                    return "ingestion";
                case subsystem_t::etl:
                    return "etl";
                case subsystem_t::stream_processing:
                    return "stream_processing";
                case subsystem_t::batch_processing:
                    return "batch_processing";
                case subsystem_t::data_warehouse:
                    return "data_warehouse";
                case subsystem_t::data_lake:
                    return "data_lake";
                case subsystem_t::olap:
                    return "olap";
                case subsystem_t::data_mining:
                    return "data_mining";
                case subsystem_t::machine_learning:
                    return "machine_learning";
                case subsystem_t::graph_compute:
                    return "graph_compute";
                case subsystem_t::realtime:
                    return "realtime";
                case subsystem_t::flink:
                    return "flink";
                case subsystem_t::spark:
                    return "spark";
                case subsystem_t::hive:
                    return "hive";
                case subsystem_t::presto:
                    return "presto";
                case subsystem_t::kafka:
                    return "kafka";
                case subsystem_t::data_governance:
                    return "data_governance";
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
                case utils::string_utils_t::hash("ingestion"):
                    return subsystem_t::ingestion;
                case utils::string_utils_t::hash("etl"):
                    return subsystem_t::etl;
                case utils::string_utils_t::hash("stream_processing"):
                    return subsystem_t::stream_processing;
                case utils::string_utils_t::hash("batch_processing"):
                    return subsystem_t::batch_processing;
                case utils::string_utils_t::hash("data_warehouse"):
                    return subsystem_t::data_warehouse;
                case utils::string_utils_t::hash("data_lake"):
                    return subsystem_t::data_lake;
                case utils::string_utils_t::hash("olap"):
                    return subsystem_t::olap;
                case utils::string_utils_t::hash("data_mining"):
                    return subsystem_t::data_mining;
                case utils::string_utils_t::hash("machine_learning"):
                    return subsystem_t::machine_learning;
                case utils::string_utils_t::hash("graph_compute"):
                    return subsystem_t::graph_compute;
                case utils::string_utils_t::hash("realtime"):
                    return subsystem_t::realtime;
                case utils::string_utils_t::hash("flink"):
                    return subsystem_t::flink;
                case utils::string_utils_t::hash("spark"):
                    return subsystem_t::spark;
                case utils::string_utils_t::hash("hive"):
                    return subsystem_t::hive;
                case utils::string_utils_t::hash("presto"):
                    return subsystem_t::presto;
                case utils::string_utils_t::hash("kafka"):
                    return subsystem_t::kafka;
                case utils::string_utils_t::hash("data_governance"):
                    return subsystem_t::data_governance;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
