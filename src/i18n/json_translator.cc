#include "error_system/i18n/json_translator.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/utils/string_utils.h"
#include "error_system/traits/subsystem_dispatcher.h"
#include "error_system/traits/module_dispatcher.h"

namespace error_system::i18n {
    /**
     * @brief 初始化 JSON 字典
     * @details 从 JSON 字典文件中加载 JSON 字典
     * @return bool 是否成功加载 JSON 字典
     */
    bool json_translator_t::__initialized_json_dict() noexcept {
        std::string lang_str = to_string(language_);
        std::string path = "include/error_system/i18n/languages/" + lang_str + ".json";
        
        auto dict_opt = utils::json_dict_t::from_file(path);
        if (dict_opt) {
            json_dict_ = std::move(*dict_opt);
            return true;
        }
        return false;
    }

    /**
     * @brief 构造函数
     * @param language 翻译器默认语言
     */
    json_translator_t::json_translator_t(language_t language) noexcept : language_(language), json_dict_{} {
        __initialized_json_dict();
    }

    /**
     * @brief 翻译错误码
     * @details 将错误码翻译为指定语言的文本
     * @param code 错误码
     * @return std::string 翻译后的文本
     */
    std::string json_translator_t::translate(const core::error_code_t& code) const noexcept {
        if (json_dict_.empty()) {
            return "Translator not initialized or dict missing";
        }

        std::string level_str  = core::to_string(code.get_level());
        std::string domain_str = domain::to_string(code.get_system());
        std::string subsys_str = traits::resolve_subsystem(code.get_subsys());
        std::string module_str = traits::resolve_module(code.get_module());

        std::string level_trans  = json_dict_.get_value_or("error_level." + level_str, level_str).value();
        std::string domain_trans = json_dict_.get_value_or("domain." + domain_str, domain_str).value();
        std::string subsys_trans = json_dict_.get_value_or("subsystem." + domain_str + "." + subsys_str, subsys_str).value();
        std::string module_trans = json_dict_.get_value_or("module." + domain_str + "." + module_str, module_str).value();

        return utils::string_utils_t::format(
            format_,
            level_trans, domain_trans, subsys_trans, module_trans, code.get_number()
        );
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
        __initialized_json_dict();
    }

    /**
     * @brief 获取翻译格式
     * @return const std::string& 翻译格式
     */
    const std::string& json_translator_t::get_format() const noexcept  {
        return format_;
    }

    /**
     * @brief 设置翻译格式
     * @param format 翻译格式
     */
    void json_translator_t::set_format(const std::string& format) noexcept {
        format_ = format;
    }

}  // namespace error_system::i18n
