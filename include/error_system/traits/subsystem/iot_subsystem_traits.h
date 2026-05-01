#pragma once
#include "error_system/subsystem/iot_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::iot_subsystem_t, void> {

        using subsystem_t = subsystem::iot_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::industrial_iot);
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
                case subsystem_t::device:
                    return "device";
                case subsystem_t::sensor:
                    return "sensor";
                case subsystem_t::actuator:
                    return "actuator";
                case subsystem_t::telemetry:
                    return "telemetry";
                case subsystem_t::command:
                    return "command";
                case subsystem_t::firmware:
                    return "firmware";
                case subsystem_t::ota:
                    return "ota";
                case subsystem_t::digital_twin:
                    return "digital_twin";
                case subsystem_t::mqtt_broker:
                    return "mqtt_broker";
                case subsystem_t::lorawan:
                    return "lorawan";
                case subsystem_t::nb_iot:
                    return "nb_iot";
                case subsystem_t::zigbee_hub:
                    return "zigbee_hub";
                case subsystem_t::ble:
                    return "ble";
                case subsystem_t::rfid:
                    return "rfid";
                case subsystem_t::gps:
                    return "gps";
                case subsystem_t::smart_home:
                    return "smart_home";
                case subsystem_t::industrial_iot:
                    return "industrial_iot";
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
                case utils::string_utils_t::hash("device"):
                    return subsystem_t::device;
                case utils::string_utils_t::hash("sensor"):
                    return subsystem_t::sensor;
                case utils::string_utils_t::hash("actuator"):
                    return subsystem_t::actuator;
                case utils::string_utils_t::hash("telemetry"):
                    return subsystem_t::telemetry;
                case utils::string_utils_t::hash("command"):
                    return subsystem_t::command;
                case utils::string_utils_t::hash("firmware"):
                    return subsystem_t::firmware;
                case utils::string_utils_t::hash("ota"):
                    return subsystem_t::ota;
                case utils::string_utils_t::hash("digital_twin"):
                    return subsystem_t::digital_twin;
                case utils::string_utils_t::hash("mqtt_broker"):
                    return subsystem_t::mqtt_broker;
                case utils::string_utils_t::hash("lorawan"):
                    return subsystem_t::lorawan;
                case utils::string_utils_t::hash("nb_iot"):
                    return subsystem_t::nb_iot;
                case utils::string_utils_t::hash("zigbee_hub"):
                    return subsystem_t::zigbee_hub;
                case utils::string_utils_t::hash("ble"):
                    return subsystem_t::ble;
                case utils::string_utils_t::hash("rfid"):
                    return subsystem_t::rfid;
                case utils::string_utils_t::hash("gps"):
                    return subsystem_t::gps;
                case utils::string_utils_t::hash("smart_home"):
                    return subsystem_t::smart_home;
                case utils::string_utils_t::hash("industrial_iot"):
                    return subsystem_t::industrial_iot;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
