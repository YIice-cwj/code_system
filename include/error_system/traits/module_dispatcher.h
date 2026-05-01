#pragma once

/**
 * @file module_dispatcher.h
 * @brief Module dispatcher for metadata resolution
 * @details 自动生成的 module 分发器，用于根据数值范围或类型字符串解析元数据
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include <cstdint>
#include "error_system/module/module.h"
#include "error_system/traits/module/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief 根据数值解析 module 名称
     * @param value module 的整数值
     * @return const char* 对应的名称字符串，若无匹配返回 "none"
     */
    static constexpr const char* resolve_module(uint16_t value) noexcept {
        if (traits::module_traits<module::ai_module_t>::is_valid(value))
            return traits::module_traits<module::ai_module_t>::to_string(static_cast<module::ai_module_t>(value));
        if (traits::module_traits<module::cache_module_t>::is_valid(value))
            return traits::module_traits<module::cache_module_t>::to_string(static_cast<module::cache_module_t>(value));
        if (traits::module_traits<module::common_module_t>::is_valid(value))
            return traits::module_traits<module::common_module_t>::to_string(static_cast<module::common_module_t>(value));
        if (traits::module_traits<module::compute_module_t>::is_valid(value))
            return traits::module_traits<module::compute_module_t>::to_string(static_cast<module::compute_module_t>(value));
        if (traits::module_traits<module::config_module_t>::is_valid(value))
            return traits::module_traits<module::config_module_t>::to_string(static_cast<module::config_module_t>(value));
        if (traits::module_traits<module::database_module_t>::is_valid(value))
            return traits::module_traits<module::database_module_t>::to_string(static_cast<module::database_module_t>(value));
        if (traits::module_traits<module::log_module_t>::is_valid(value))
            return traits::module_traits<module::log_module_t>::to_string(static_cast<module::log_module_t>(value));
        if (traits::module_traits<module::message_module_t>::is_valid(value))
            return traits::module_traits<module::message_module_t>::to_string(static_cast<module::message_module_t>(value));
        if (traits::module_traits<module::monitor_module_t>::is_valid(value))
            return traits::module_traits<module::monitor_module_t>::to_string(static_cast<module::monitor_module_t>(value));
        if (traits::module_traits<module::network_module_t>::is_valid(value))
            return traits::module_traits<module::network_module_t>::to_string(static_cast<module::network_module_t>(value));
        if (traits::module_traits<module::security_module_t>::is_valid(value))
            return traits::module_traits<module::security_module_t>::to_string(static_cast<module::security_module_t>(value));
        if (traits::module_traits<module::storage_module_t>::is_valid(value))
            return traits::module_traits<module::storage_module_t>::to_string(static_cast<module::storage_module_t>(value));
        return "none";
    }

    /**
     * @brief 根据类型和名称解析 module 枚举值
     * @param type module 的分类名称
     * @param name module 的具体名称
     * @return uint16_t 对应的枚举整数值，若无匹配返回 0
     */
    static constexpr uint16_t module_from_string(const char* type, const char* name) noexcept {
        switch (utils::string_utils_t::hash(type)) {
            case utils::string_utils_t::hash("ai"):
                return static_cast<uint16_t>(traits::module_traits<module::ai_module_t>::from_string(name));
            case utils::string_utils_t::hash("cache"):
                return static_cast<uint16_t>(traits::module_traits<module::cache_module_t>::from_string(name));
            case utils::string_utils_t::hash("common"):
                return static_cast<uint16_t>(traits::module_traits<module::common_module_t>::from_string(name));
            case utils::string_utils_t::hash("compute"):
                return static_cast<uint16_t>(traits::module_traits<module::compute_module_t>::from_string(name));
            case utils::string_utils_t::hash("config"):
                return static_cast<uint16_t>(traits::module_traits<module::config_module_t>::from_string(name));
            case utils::string_utils_t::hash("database"):
                return static_cast<uint16_t>(traits::module_traits<module::database_module_t>::from_string(name));
            case utils::string_utils_t::hash("log"):
                return static_cast<uint16_t>(traits::module_traits<module::log_module_t>::from_string(name));
            case utils::string_utils_t::hash("message"):
                return static_cast<uint16_t>(traits::module_traits<module::message_module_t>::from_string(name));
            case utils::string_utils_t::hash("monitor"):
                return static_cast<uint16_t>(traits::module_traits<module::monitor_module_t>::from_string(name));
            case utils::string_utils_t::hash("network"):
                return static_cast<uint16_t>(traits::module_traits<module::network_module_t>::from_string(name));
            case utils::string_utils_t::hash("security"):
                return static_cast<uint16_t>(traits::module_traits<module::security_module_t>::from_string(name));
            case utils::string_utils_t::hash("storage"):
                return static_cast<uint16_t>(traits::module_traits<module::storage_module_t>::from_string(name));
            default: return 0;
        }
    }

}  // namespace error_system::traits