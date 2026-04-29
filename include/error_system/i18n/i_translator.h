#pragma once
#include "error_system/i18n/language.h"
#include <string>

namespace error_system::core {
    class error_code_t;
}  // namespace error_system::core

/**
 * @file i_translator.h
 * @brief 翻译器接口
 * @details 定义翻译器相关的接口，用于将错误码翻译为指定语言的文本
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::i18n {

    /**
     * @brief 翻译器接口
     * @details 定义翻译器相关的接口，用于将错误码翻译为指定语言的文本
     */
    class i_translator_t {
        public:
        /**
         * @brief 析构函数
         */
        virtual ~i_translator_t() = default;

        /**
         * @brief 翻译错误码
         * @details 将错误码翻译为当前设置语言的文本
         * @param code 错误码
         * @return std::string 翻译后的文本
         */
        virtual std::string translate(const core::error_code_t& code) const noexcept = 0;

        /**
         * @brief 获取翻译器当前设置的语言
         * @return language_t 当前语言
         */
        virtual language_t get_language() const noexcept = 0;

        /**
         * @brief 设置翻译器当前语言
         * @param lang 目标语言
         */
        virtual void set_language(language_t lang) noexcept = 0;
    };

}  // namespace error_system::i18n