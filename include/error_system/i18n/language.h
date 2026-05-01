#pragma once
#include "error_system/utils/string_utils.h"
#include <string>
#include <cstdint>

/**
 * @file language.h
 * @brief 语言枚举
 * @details 定义语言相关的枚举值，用于表示不同的语言
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::i18n {

    /**
     * @brief 语言枚举
     * @details 定义语言相关的枚举值，用于表示不同的语言
     */
    enum class language_t : uint8_t {
        zh_CN = 0,  // 中文
        en_US = 1   // 英文
    };

    /**
     * @brief 语言字符串映射
     * @details 用于将语言枚举值转换为语言字符串
     */
    constexpr const char* const LANGUAGE_STRING[] = {"zh_CN", "en_US"};

    /**
     * @brief 语言枚举值转换为语言整数
     * @details 用于将语言枚举值转换为语言整数
     * @param lang 语言枚举值
     * @return uint8_t 语言整数
     */
    constexpr uint8_t to_int(language_t lang) noexcept {
        return static_cast<uint8_t>(std::underlying_type_t<language_t>(lang));
    }

    /**
     * @brief 语言枚举值是否有效
     * @details 用于判断语言枚举值是否有效
     * @param lang 语言枚举值
     * @return bool 语言枚举值是否有效
     */
    constexpr bool is_valid(uint8_t lang) noexcept {
        return lang <= to_int(language_t::en_US);
    }

    /**
     * @brief 语言整数转换为语言枚举值
     * @details 用于将语言整数转换为语言枚举值
     * @param lang 语言整数
     * @return language_t 语言枚举值
     */
    constexpr language_t from_int(uint8_t lang) noexcept {
        if (!is_valid(lang)) {
            return language_t::zh_CN;
        }
        return static_cast<language_t>(lang);
    }

    /**
     * @brief 语言字符串转换为语言枚举值
     * @details 用于将语言字符串转换为语言枚举值
     * @param lang 语言字符串
     * @return language_t 语言枚举值
     */
    constexpr language_t from_string(const std::string& lang) noexcept {
        switch (utils::string_utils_t::hash(lang.c_str())) {
            case utils::string_utils_t::hash("zh_CN"):
                return language_t::zh_CN;
            case utils::string_utils_t::hash("en_US"):
                return language_t::en_US;
            default:
                return language_t::zh_CN;
        }
    }

    /**
     * @brief 语言字符串
     * @details 用于将语言枚举值转换为语言字符串
     * @param lang 语言枚举值
     * @return const char* 语言字符串
     */
    constexpr const char* to_string(language_t lang) noexcept {
        return LANGUAGE_STRING[to_int(lang)];
    }

}  // namespace error_system::i18n
