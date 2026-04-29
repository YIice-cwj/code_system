#pragma once
#include "error_system/i18n/i_translator.h"
/**
 * @file json_translator.h
 * @brief JSON 翻译器
 * @details 定义 JSON 翻译器相关的接口，用于将 JSON 字符串翻译为指定语言的文本
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::i18n {
    /**
     * @brief JSON 翻译器接口
     * @details 定义 JSON 翻译器相关的接口，用于将 JSON 字符串翻译为指定语言的文本
     */
    class json_translator_t : public i_translator_t {
        private:
        language_t language_ = language_t::zh_CN;

        public:
        /**
         * @brief 构造函数
         */
        json_translator_t() = default;

        /**
         * @brief 析构函数
         */
        virtual ~json_translator_t() = default;

        /**
         * @brief 翻译错误码
         * @details 将错误码翻译为指定语言的文本
         * @param code 错误码
         * @return std::string 翻译后的文本
         */
        std::string translate(const core::error_code_t& code) const noexcept override;

        /**
         * @brief 获取翻译器当前设置的语言
         * @return language_t 当前语言
         */
        language_t get_language() const noexcept override;

        /**
         * @brief 设置翻译器当前语言
         * @param lang 目标语言
         */
        void set_language(language_t lang) noexcept override;
    };
}  // namespace error_system::i18n
