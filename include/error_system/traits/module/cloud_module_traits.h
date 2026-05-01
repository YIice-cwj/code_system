#pragma once
#include "error_system/module/cloud_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::cloud_module_t, void> {

        using module_t = module::cloud_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::policy_engine);
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
                case module_t::instance_manager:
                    return "instance_manager";
                case module_t::image_manager:
                    return "image_manager";
                case module_t::network_manager:
                    return "network_manager";
                case module_t::volume_manager:
                    return "volume_manager";
                case module_t::snapshot_manager:
                    return "snapshot_manager";
                case module_t::template_manager:
                    return "template_manager";
                case module_t::scheduler:
                    return "scheduler";
                case module_t::scaler:
                    return "scaler";
                case module_t::load_balancer:
                    return "load_balancer";
                case module_t::health_checker:
                    return "health_checker";
                case module_t::metadata_service:
                    return "metadata_service";
                case module_t::tag_manager:
                    return "tag_manager";
                case module_t::quota_manager:
                    return "quota_manager";
                case module_t::billing_meter:
                    return "billing_meter";
                case module_t::event_notifier:
                    return "event_notifier";
                case module_t::api_controller:
                    return "api_controller";
                case module_t::policy_engine:
                    return "policy_engine";
                default:
                    return "none";
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
                case utils::string_utils_t::hash("instance_manager"):
                    return module_t::instance_manager;
                case utils::string_utils_t::hash("image_manager"):
                    return module_t::image_manager;
                case utils::string_utils_t::hash("network_manager"):
                    return module_t::network_manager;
                case utils::string_utils_t::hash("volume_manager"):
                    return module_t::volume_manager;
                case utils::string_utils_t::hash("snapshot_manager"):
                    return module_t::snapshot_manager;
                case utils::string_utils_t::hash("template_manager"):
                    return module_t::template_manager;
                case utils::string_utils_t::hash("scheduler"):
                    return module_t::scheduler;
                case utils::string_utils_t::hash("scaler"):
                    return module_t::scaler;
                case utils::string_utils_t::hash("load_balancer"):
                    return module_t::load_balancer;
                case utils::string_utils_t::hash("health_checker"):
                    return module_t::health_checker;
                case utils::string_utils_t::hash("metadata_service"):
                    return module_t::metadata_service;
                case utils::string_utils_t::hash("tag_manager"):
                    return module_t::tag_manager;
                case utils::string_utils_t::hash("quota_manager"):
                    return module_t::quota_manager;
                case utils::string_utils_t::hash("billing_meter"):
                    return module_t::billing_meter;
                case utils::string_utils_t::hash("event_notifier"):
                    return module_t::event_notifier;
                case utils::string_utils_t::hash("api_controller"):
                    return module_t::api_controller;
                case utils::string_utils_t::hash("policy_engine"):
                    return module_t::policy_engine;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
