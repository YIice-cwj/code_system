#pragma once

/**
 * @file message_module_traits.h
 * @brief Module message traits specialization
 * @details 自动生成的 module message traits 特化，用于提供枚举值的元数据和转换功能
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include "error_system/module/message_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief module::message_module_t 的 traits 特化
     * @details 提供 message_module_t 与整数及字符串之间的转换和校验
     */
    template <>
    struct module_traits<module::message_module_t, void> {

        /** @brief 关联的枚举类型 */
        using module_t = module::message_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::ack_manager);
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
                case module_t::producer: return "producer";
                case module_t::consumer: return "consumer";
                case module_t::broker: return "broker";
                case module_t::exchange: return "exchange";
                case module_t::queue_manager: return "queue_manager";
                case module_t::topic_manager: return "topic_manager";
                case module_t::partitioner: return "partitioner";
                case module_t::router: return "router";
                case module_t::serializer: return "serializer";
                case module_t::deserializer: return "deserializer";
                case module_t::compressor: return "compressor";
                case module_t::idempotency: return "idempotency";
                case module_t::deduplicator: return "deduplicator";
                case module_t::delay_scheduler: return "delay_scheduler";
                case module_t::dead_letter: return "dead_letter";
                case module_t::transaction: return "transaction";
                case module_t::ack_manager: return "ack_manager";
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
                case utils::string_utils_t::hash("producer"): return module_t::producer;
                case utils::string_utils_t::hash("consumer"): return module_t::consumer;
                case utils::string_utils_t::hash("broker"): return module_t::broker;
                case utils::string_utils_t::hash("exchange"): return module_t::exchange;
                case utils::string_utils_t::hash("queue_manager"): return module_t::queue_manager;
                case utils::string_utils_t::hash("topic_manager"): return module_t::topic_manager;
                case utils::string_utils_t::hash("partitioner"): return module_t::partitioner;
                case utils::string_utils_t::hash("router"): return module_t::router;
                case utils::string_utils_t::hash("serializer"): return module_t::serializer;
                case utils::string_utils_t::hash("deserializer"): return module_t::deserializer;
                case utils::string_utils_t::hash("compressor"): return module_t::compressor;
                case utils::string_utils_t::hash("idempotency"): return module_t::idempotency;
                case utils::string_utils_t::hash("deduplicator"): return module_t::deduplicator;
                case utils::string_utils_t::hash("delay_scheduler"): return module_t::delay_scheduler;
                case utils::string_utils_t::hash("dead_letter"): return module_t::dead_letter;
                case utils::string_utils_t::hash("transaction"): return module_t::transaction;
                case utils::string_utils_t::hash("ack_manager"): return module_t::ack_manager;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits