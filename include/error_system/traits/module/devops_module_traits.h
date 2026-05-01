#pragma once
#include "error_system/module/devops_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::devops_module_t, void> {

        using module_t = module::devops_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::artifact_registry);
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
                case module_t::scm_connector:
                    return "scm_connector";
                case module_t::build_executor:
                    return "build_executor";
                case module_t::test_runner:
                    return "test_runner";
                case module_t::package_builder:
                    return "package_builder";
                case module_t::deploy_engine:
                    return "deploy_engine";
                case module_t::rollback_manager:
                    return "rollback_manager";
                case module_t::environment_manager:
                    return "environment_manager";
                case module_t::secret_manager:
                    return "secret_manager";
                case module_t::alert_manager:
                    return "alert_manager";
                case module_t::log_aggregator:
                    return "log_aggregator";
                case module_t::trace_collector:
                    return "trace_collector";
                case module_t::metric_scraper:
                    return "metric_scraper";
                case module_t::slo_calculator:
                    return "slo_calculator";
                case module_t::chaos_injector:
                    return "chaos_injector";
                case module_t::git_syncer:
                    return "git_syncer";
                case module_t::provisioner:
                    return "provisioner";
                case module_t::artifact_registry:
                    return "artifact_registry";
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
                case utils::string_utils_t::hash("scm_connector"):
                    return module_t::scm_connector;
                case utils::string_utils_t::hash("build_executor"):
                    return module_t::build_executor;
                case utils::string_utils_t::hash("test_runner"):
                    return module_t::test_runner;
                case utils::string_utils_t::hash("package_builder"):
                    return module_t::package_builder;
                case utils::string_utils_t::hash("deploy_engine"):
                    return module_t::deploy_engine;
                case utils::string_utils_t::hash("rollback_manager"):
                    return module_t::rollback_manager;
                case utils::string_utils_t::hash("environment_manager"):
                    return module_t::environment_manager;
                case utils::string_utils_t::hash("secret_manager"):
                    return module_t::secret_manager;
                case utils::string_utils_t::hash("alert_manager"):
                    return module_t::alert_manager;
                case utils::string_utils_t::hash("log_aggregator"):
                    return module_t::log_aggregator;
                case utils::string_utils_t::hash("trace_collector"):
                    return module_t::trace_collector;
                case utils::string_utils_t::hash("metric_scraper"):
                    return module_t::metric_scraper;
                case utils::string_utils_t::hash("slo_calculator"):
                    return module_t::slo_calculator;
                case utils::string_utils_t::hash("chaos_injector"):
                    return module_t::chaos_injector;
                case utils::string_utils_t::hash("git_syncer"):
                    return module_t::git_syncer;
                case utils::string_utils_t::hash("provisioner"):
                    return module_t::provisioner;
                case utils::string_utils_t::hash("artifact_registry"):
                    return module_t::artifact_registry;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
