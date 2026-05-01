#pragma once
#include "error_system/module/application_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::application_module_t, void> {

        using module_t = module::application_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::shader);
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
                case module_t::controller:
                    return "controller";
                case module_t::service:
                    return "service";
                case module_t::repository:
                    return "repository";
                case module_t::dto:
                    return "dto";
                case module_t::view:
                    return "view";
                case module_t::template_engine:
                    return "template_engine";
                case module_t::form:
                    return "form";
                case module_t::validator:
                    return "validator";
                case module_t::converter:
                    return "converter";
                case module_t::interceptor:
                    return "interceptor";
                case module_t::filter:
                    return "filter";
                case module_t::listener:
                    return "listener";
                case module_t::resolver:
                    return "resolver";
                case module_t::router:
                    return "router";
                case module_t::middleware:
                    return "middleware";
                case module_t::serializer:
                    return "serializer";
                case module_t::deserializer:
                    return "deserializer";
                case module_t::paginator:
                    return "paginator";
                case module_t::exporter:
                    return "exporter";
                case module_t::importer:
                    return "importer";
                case module_t::renderer:
                    return "renderer";
                case module_t::animator:
                    return "animator";
                case module_t::physics:
                    return "physics";
                case module_t::ai_model:
                    return "ai_model";
                case module_t::shader:
                    return "shader";
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
                case utils::string_utils_t::hash("controller"):
                    return module_t::controller;
                case utils::string_utils_t::hash("service"):
                    return module_t::service;
                case utils::string_utils_t::hash("repository"):
                    return module_t::repository;
                case utils::string_utils_t::hash("dto"):
                    return module_t::dto;
                case utils::string_utils_t::hash("view"):
                    return module_t::view;
                case utils::string_utils_t::hash("template_engine"):
                    return module_t::template_engine;
                case utils::string_utils_t::hash("form"):
                    return module_t::form;
                case utils::string_utils_t::hash("validator"):
                    return module_t::validator;
                case utils::string_utils_t::hash("converter"):
                    return module_t::converter;
                case utils::string_utils_t::hash("interceptor"):
                    return module_t::interceptor;
                case utils::string_utils_t::hash("filter"):
                    return module_t::filter;
                case utils::string_utils_t::hash("listener"):
                    return module_t::listener;
                case utils::string_utils_t::hash("resolver"):
                    return module_t::resolver;
                case utils::string_utils_t::hash("router"):
                    return module_t::router;
                case utils::string_utils_t::hash("middleware"):
                    return module_t::middleware;
                case utils::string_utils_t::hash("serializer"):
                    return module_t::serializer;
                case utils::string_utils_t::hash("deserializer"):
                    return module_t::deserializer;
                case utils::string_utils_t::hash("paginator"):
                    return module_t::paginator;
                case utils::string_utils_t::hash("exporter"):
                    return module_t::exporter;
                case utils::string_utils_t::hash("importer"):
                    return module_t::importer;
                case utils::string_utils_t::hash("renderer"):
                    return module_t::renderer;
                case utils::string_utils_t::hash("animator"):
                    return module_t::animator;
                case utils::string_utils_t::hash("physics"):
                    return module_t::physics;
                case utils::string_utils_t::hash("ai_model"):
                    return module_t::ai_model;
                case utils::string_utils_t::hash("shader"):
                    return module_t::shader;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
