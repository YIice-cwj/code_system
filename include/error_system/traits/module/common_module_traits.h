#pragma once
#include "error_system/module/common_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::common_module_t, void> {

        using module_t = module::common_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::logger);
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
                case module_t::connector:
                    return "connector";
                case module_t::parser:
                    return "parser";
                case module_t::serializer:
                    return "serializer";
                case module_t::validator:
                    return "validator";
                case module_t::transformer:
                    return "transformer";
                case module_t::encoder:
                    return "encoder";
                case module_t::decoder:
                    return "decoder";
                case module_t::compressor:
                    return "compressor";
                case module_t::encryptor:
                    return "encryptor";
                case module_t::pool:
                    return "pool";
                case module_t::cache:
                    return "cache";
                case module_t::router:
                    return "router";
                case module_t::handler:
                    return "handler";
                case module_t::interceptor:
                    return "interceptor";
                case module_t::filter:
                    return "filter";
                case module_t::adapter:
                    return "adapter";
                case module_t::proxy:
                    return "proxy";
                case module_t::balancer:
                    return "balancer";
                case module_t::watcher:
                    return "watcher";
                case module_t::scheduler:
                    return "scheduler";
                case module_t::throttler:
                    return "throttler";
                case module_t::retryer:
                    return "retryer";
                case module_t::circuit_breaker:
                    return "circuit_breaker";
                case module_t::health_checker:
                    return "health_checker";
                case module_t::discovery:
                    return "discovery";
                case module_t::registry:
                    return "registry";
                case module_t::config_loader:
                    return "config_loader";
                case module_t::event_bus:
                    return "event_bus";
                case module_t::command_bus:
                    return "command_bus";
                case module_t::query_bus:
                    return "query_bus";
                case module_t::saga:
                    return "saga";
                case module_t::outbox:
                    return "outbox";
                case module_t::inbox:
                    return "inbox";
                case module_t::idempotency:
                    return "idempotency";
                case module_t::audit:
                    return "audit";
                case module_t::meter:
                    return "meter";
                case module_t::tracer:
                    return "tracer";
                case module_t::logger:
                    return "logger";
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
                case utils::string_utils_t::hash("connector"):
                    return module_t::connector;
                case utils::string_utils_t::hash("parser"):
                    return module_t::parser;
                case utils::string_utils_t::hash("serializer"):
                    return module_t::serializer;
                case utils::string_utils_t::hash("validator"):
                    return module_t::validator;
                case utils::string_utils_t::hash("transformer"):
                    return module_t::transformer;
                case utils::string_utils_t::hash("encoder"):
                    return module_t::encoder;
                case utils::string_utils_t::hash("decoder"):
                    return module_t::decoder;
                case utils::string_utils_t::hash("compressor"):
                    return module_t::compressor;
                case utils::string_utils_t::hash("encryptor"):
                    return module_t::encryptor;
                case utils::string_utils_t::hash("pool"):
                    return module_t::pool;
                case utils::string_utils_t::hash("cache"):
                    return module_t::cache;
                case utils::string_utils_t::hash("router"):
                    return module_t::router;
                case utils::string_utils_t::hash("handler"):
                    return module_t::handler;
                case utils::string_utils_t::hash("interceptor"):
                    return module_t::interceptor;
                case utils::string_utils_t::hash("filter"):
                    return module_t::filter;
                case utils::string_utils_t::hash("adapter"):
                    return module_t::adapter;
                case utils::string_utils_t::hash("proxy"):
                    return module_t::proxy;
                case utils::string_utils_t::hash("balancer"):
                    return module_t::balancer;
                case utils::string_utils_t::hash("watcher"):
                    return module_t::watcher;
                case utils::string_utils_t::hash("scheduler"):
                    return module_t::scheduler;
                case utils::string_utils_t::hash("throttler"):
                    return module_t::throttler;
                case utils::string_utils_t::hash("retryer"):
                    return module_t::retryer;
                case utils::string_utils_t::hash("circuit_breaker"):
                    return module_t::circuit_breaker;
                case utils::string_utils_t::hash("health_checker"):
                    return module_t::health_checker;
                case utils::string_utils_t::hash("discovery"):
                    return module_t::discovery;
                case utils::string_utils_t::hash("registry"):
                    return module_t::registry;
                case utils::string_utils_t::hash("config_loader"):
                    return module_t::config_loader;
                case utils::string_utils_t::hash("event_bus"):
                    return module_t::event_bus;
                case utils::string_utils_t::hash("command_bus"):
                    return module_t::command_bus;
                case utils::string_utils_t::hash("query_bus"):
                    return module_t::query_bus;
                case utils::string_utils_t::hash("saga"):
                    return module_t::saga;
                case utils::string_utils_t::hash("outbox"):
                    return module_t::outbox;
                case utils::string_utils_t::hash("inbox"):
                    return module_t::inbox;
                case utils::string_utils_t::hash("idempotency"):
                    return module_t::idempotency;
                case utils::string_utils_t::hash("audit"):
                    return module_t::audit;
                case utils::string_utils_t::hash("meter"):
                    return module_t::meter;
                case utils::string_utils_t::hash("tracer"):
                    return module_t::tracer;
                case utils::string_utils_t::hash("logger"):
                    return module_t::logger;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
