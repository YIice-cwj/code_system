#pragma once
#include "error_system/module/iot_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::iot_module_t, void> {

        using module_t = module::iot_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::shadow_manager);
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
                case module_t::device_registry:
                    return "device_registry";
                case module_t::sensor_reader:
                    return "sensor_reader";
                case module_t::actuator_controller:
                    return "actuator_controller";
                case module_t::telemetry_collector:
                    return "telemetry_collector";
                case module_t::command_dispatcher:
                    return "command_dispatcher";
                case module_t::firmware_updater:
                    return "firmware_updater";
                case module_t::twin_manager:
                    return "twin_manager";
                case module_t::broker_connector:
                    return "broker_connector";
                case module_t::protocol_handler:
                    return "protocol_handler";
                case module_t::data_aggregator:
                    return "data_aggregator";
                case module_t::alarm_generator:
                    return "alarm_generator";
                case module_t::geofence_engine:
                    return "geofence_engine";
                case module_t::energy_manager:
                    return "energy_manager";
                case module_t::shadow_manager:
                    return "shadow_manager";
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
                case utils::string_utils_t::hash("device_registry"):
                    return module_t::device_registry;
                case utils::string_utils_t::hash("sensor_reader"):
                    return module_t::sensor_reader;
                case utils::string_utils_t::hash("actuator_controller"):
                    return module_t::actuator_controller;
                case utils::string_utils_t::hash("telemetry_collector"):
                    return module_t::telemetry_collector;
                case utils::string_utils_t::hash("command_dispatcher"):
                    return module_t::command_dispatcher;
                case utils::string_utils_t::hash("firmware_updater"):
                    return module_t::firmware_updater;
                case utils::string_utils_t::hash("twin_manager"):
                    return module_t::twin_manager;
                case utils::string_utils_t::hash("broker_connector"):
                    return module_t::broker_connector;
                case utils::string_utils_t::hash("protocol_handler"):
                    return module_t::protocol_handler;
                case utils::string_utils_t::hash("data_aggregator"):
                    return module_t::data_aggregator;
                case utils::string_utils_t::hash("alarm_generator"):
                    return module_t::alarm_generator;
                case utils::string_utils_t::hash("geofence_engine"):
                    return module_t::geofence_engine;
                case utils::string_utils_t::hash("energy_manager"):
                    return module_t::energy_manager;
                case utils::string_utils_t::hash("shadow_manager"):
                    return module_t::shadow_manager;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
