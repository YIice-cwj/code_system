#include "error_system/i18n/translator_registry.h"
#include "error_system/i18n/json_translator.h"

namespace error_system::i18n {

    /**
     * @brief 获取单例实例
     * @return translator_registry_t& 单例引用
     */
    translator_registry_t& translator_registry_t::instance() noexcept {
        static translator_registry_t instance;
        return instance;
    }

    /**
     * @brief 注册全局翻译器
     * @param translator 翻译器指针，传入 nullptr 可清除注册
     */
    void translator_registry_t::set(i_translator_t* translator) noexcept {
        translator_ = translator;
    }

    /**
     * @brief 获取当前全局翻译器
     * @details 若未注册，自动创建并注册默认中文翻译器
     * @return i_translator_t* 全局翻译器指针
     */
    i_translator_t* translator_registry_t::get() const noexcept {
        if (!translator_) {
            static json_translator_t default_translator(language_t::zh_cn);
            translator_ = &default_translator;
        }
        return translator_;
    }

    /**
     * @brief 判断是否已注册全局翻译器
     * @return bool 是否已注册
     */
    bool translator_registry_t::has_translator() const noexcept {
        return translator_ != nullptr;
    }

}  // namespace error_system::i18n
