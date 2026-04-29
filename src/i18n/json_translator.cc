#include "error_system/i18n/json_translator.h"

namespace error_system::i18n {

    /**
     * @brief 翻译错误码
     * @details 将错误码翻译为指定语言的文本
     * @param code 错误码
     * @return std::string 翻译后的文本
     */
    std::string json_translator_t::translate(const core::error_code_t& code) const noexcept {
        (void)code;
        return "";
    }

    /**
     * @brief 获取翻译器当前设置的语言
     * @return language_t 当前语言
     */
    language_t json_translator_t::get_language() const noexcept {
        return language_;
    }

    /**
     * @brief 设置翻译器当前语言
     * @param lang 目标语言
     */
    void json_translator_t::set_language(language_t lang) noexcept {
        language_ = lang;
    }

}  // namespace error_system::i18n