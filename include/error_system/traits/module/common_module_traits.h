#pragma once

/**
 * @file common_module_traits.h
 * @brief Module common traits specialization
 * @details 自动生成的 module common traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/common_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::common_module_t 的 traits 特化
     * @details 提供 common_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::common_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::common_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::audit);
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
                case module_t::serializer: return "serializer";
                case module_t::deserializer: return "deserializer";
                case module_t::validator: return "validator";
                case module_t::transformer: return "transformer";
                case module_t::encoder: return "encoder";
                case module_t::decoder: return "decoder";
                case module_t::compressor: return "compressor";
                case module_t::decompressor: return "decompressor";
                case module_t::pool: return "pool";
                case module_t::router: return "router";
                case module_t::handler: return "handler";
                case module_t::interceptor: return "interceptor";
                case module_t::adapter: return "adapter";
                case module_t::balancer: return "balancer";
                case module_t::throttler: return "throttler";
                case module_t::retryer: return "retryer";
                case module_t::discovery: return "discovery";
                case module_t::registry: return "registry";
                case module_t::event_bus: return "event_bus";
                case module_t::command_bus: return "command_bus";
                case module_t::query_bus: return "query_bus";
                case module_t::idempotency: return "idempotency";
                case module_t::audit: return "audit";
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
                case utils::string_utils_t::hash("serializer"): return module_t::serializer;
                case utils::string_utils_t::hash("deserializer"): return module_t::deserializer;
                case utils::string_utils_t::hash("validator"): return module_t::validator;
                case utils::string_utils_t::hash("transformer"): return module_t::transformer;
                case utils::string_utils_t::hash("encoder"): return module_t::encoder;
                case utils::string_utils_t::hash("decoder"): return module_t::decoder;
                case utils::string_utils_t::hash("compressor"): return module_t::compressor;
                case utils::string_utils_t::hash("decompressor"): return module_t::decompressor;
                case utils::string_utils_t::hash("pool"): return module_t::pool;
                case utils::string_utils_t::hash("router"): return module_t::router;
                case utils::string_utils_t::hash("handler"): return module_t::handler;
                case utils::string_utils_t::hash("interceptor"): return module_t::interceptor;
                case utils::string_utils_t::hash("adapter"): return module_t::adapter;
                case utils::string_utils_t::hash("balancer"): return module_t::balancer;
                case utils::string_utils_t::hash("throttler"): return module_t::throttler;
                case utils::string_utils_t::hash("retryer"): return module_t::retryer;
                case utils::string_utils_t::hash("discovery"): return module_t::discovery;
                case utils::string_utils_t::hash("registry"): return module_t::registry;
                case utils::string_utils_t::hash("event_bus"): return module_t::event_bus;
                case utils::string_utils_t::hash("command_bus"): return module_t::command_bus;
                case utils::string_utils_t::hash("query_bus"): return module_t::query_bus;
                case utils::string_utils_t::hash("idempotency"): return module_t::idempotency;
                case utils::string_utils_t::hash("audit"): return module_t::audit;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits