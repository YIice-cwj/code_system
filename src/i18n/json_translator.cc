#include "error_system/i18n/json_translator.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/utils/string_utils.h"
#include <cstdint>

#include "error_system/traits/subsystem/subsystem_traits.h"
#include "error_system/traits/module/module_traits.h"

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
     * @brief 根据子系统枚举值解析对应的子系统名称
     * @details 遍历所有子系统的值范围（通过 is_valid 检测），找到匹配的子系统并返回其字符串名称
     * @param subsys_val 子系统枚举值
     * @return std::string 子系统名称，若无匹配则返回 "none"
     */
    std::string json_translator_t::__resolve_subsys(uint16_t subsys_val) const noexcept {
        if (traits::subsystem_traits<subsystem::system_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::system_subsystem_t>::to_string(static_cast<subsystem::system_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::kernel_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::kernel_subsystem_t>::to_string(static_cast<subsystem::kernel_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::middleware_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::middleware_subsystem_t>::to_string(static_cast<subsystem::middleware_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::application_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::application_subsystem_t>::to_string(static_cast<subsystem::application_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::service_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::service_subsystem_t>::to_string(static_cast<subsystem::service_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::network_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::network_subsystem_t>::to_string(static_cast<subsystem::network_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::storage_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::storage_subsystem_t>::to_string(static_cast<subsystem::storage_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::database_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::database_subsystem_t>::to_string(static_cast<subsystem::database_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::security_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::security_subsystem_t>::to_string(static_cast<subsystem::security_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::ai_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::ai_subsystem_t>::to_string(static_cast<subsystem::ai_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::cloud_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::cloud_subsystem_t>::to_string(static_cast<subsystem::cloud_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::edge_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::edge_subsystem_t>::to_string(static_cast<subsystem::edge_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::iot_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::iot_subsystem_t>::to_string(static_cast<subsystem::iot_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::blockchain_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::blockchain_subsystem_t>::to_string(static_cast<subsystem::blockchain_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::bigdata_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::bigdata_subsystem_t>::to_string(static_cast<subsystem::bigdata_subsystem_t>(subsys_val));
        if (traits::subsystem_traits<subsystem::devops_subsystem_t>::is_valid(subsys_val))
            return traits::subsystem_traits<subsystem::devops_subsystem_t>::to_string(static_cast<subsystem::devops_subsystem_t>(subsys_val));
        return "none";
    }

    /**
     * @brief 根据模块枚举值解析对应的模块名称
     * @details 遍历所有模块的值范围（通过 is_valid 检测），找到匹配的模块并返回其字符串名称
     * @param module_val 模块枚举值
     * @return std::string 模块名称，若无匹配则返回 "none"
     */
    std::string json_translator_t::__resolve_module(uint16_t module_val) const noexcept {
        if (traits::module_traits<module::system_module_t>::is_valid(module_val))
            return traits::module_traits<module::system_module_t>::to_string(static_cast<module::system_module_t>(module_val));
        if (traits::module_traits<module::kernel_module_t>::is_valid(module_val))
            return traits::module_traits<module::kernel_module_t>::to_string(static_cast<module::kernel_module_t>(module_val));
        if (traits::module_traits<module::middleware_module_t>::is_valid(module_val))
            return traits::module_traits<module::middleware_module_t>::to_string(static_cast<module::middleware_module_t>(module_val));
        if (traits::module_traits<module::application_module_t>::is_valid(module_val))
            return traits::module_traits<module::application_module_t>::to_string(static_cast<module::application_module_t>(module_val));
        if (traits::module_traits<module::service_module_t>::is_valid(module_val))
            return traits::module_traits<module::service_module_t>::to_string(static_cast<module::service_module_t>(module_val));
        if (traits::module_traits<module::network_module_t>::is_valid(module_val))
            return traits::module_traits<module::network_module_t>::to_string(static_cast<module::network_module_t>(module_val));
        if (traits::module_traits<module::storage_module_t>::is_valid(module_val))
            return traits::module_traits<module::storage_module_t>::to_string(static_cast<module::storage_module_t>(module_val));
        if (traits::module_traits<module::database_module_t>::is_valid(module_val))
            return traits::module_traits<module::database_module_t>::to_string(static_cast<module::database_module_t>(module_val));
        if (traits::module_traits<module::security_module_t>::is_valid(module_val))
            return traits::module_traits<module::security_module_t>::to_string(static_cast<module::security_module_t>(module_val));
        if (traits::module_traits<module::ai_module_t>::is_valid(module_val))
            return traits::module_traits<module::ai_module_t>::to_string(static_cast<module::ai_module_t>(module_val));
        if (traits::module_traits<module::cloud_module_t>::is_valid(module_val))
            return traits::module_traits<module::cloud_module_t>::to_string(static_cast<module::cloud_module_t>(module_val));
        if (traits::module_traits<module::edge_module_t>::is_valid(module_val))
            return traits::module_traits<module::edge_module_t>::to_string(static_cast<module::edge_module_t>(module_val));
        if (traits::module_traits<module::iot_module_t>::is_valid(module_val))
            return traits::module_traits<module::iot_module_t>::to_string(static_cast<module::iot_module_t>(module_val));
        if (traits::module_traits<module::blockchain_module_t>::is_valid(module_val))
            return traits::module_traits<module::blockchain_module_t>::to_string(static_cast<module::blockchain_module_t>(module_val));
        if (traits::module_traits<module::bigdata_module_t>::is_valid(module_val))
            return traits::module_traits<module::bigdata_module_t>::to_string(static_cast<module::bigdata_module_t>(module_val));
        if (traits::module_traits<module::devops_module_t>::is_valid(module_val))
            return traits::module_traits<module::devops_module_t>::to_string(static_cast<module::devops_module_t>(module_val));
        return "none";
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
        std::string subsys_str = __resolve_subsys(code.get_subsys());
        std::string module_str = __resolve_module(code.get_module());

        std::string level_trans  = json_dict_.get_value_or("error_level." + level_str, level_str).value();
        std::string domain_trans = json_dict_.get_value_or("domain." + domain_str, domain_str).value();
        std::string subsys_trans = json_dict_.get_value_or("subsystem." + domain_str + "." + subsys_str, subsys_str).value();
        std::string module_trans = json_dict_.get_value_or("module." + domain_str + "." + module_str, module_str).value();

        return utils::string_utils_t::format(
            "[Level: {}, System: {}, Subsystem: {}, Module: {}] Code: {}",
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

}  // namespace error_system::i18n
