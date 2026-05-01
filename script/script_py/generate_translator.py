#!/usr/bin/env python3
import os

def get_project_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def main():
    root_dir = get_project_root()
    domain_file = os.path.join(root_dir, "include/error_system/domain/system_domain.h")
    
    # Extract domains
    domains = []
    with open(domain_file, 'r', encoding='utf-8') as f:
        in_enum = False
        for line in f:
            if "enum class system_domain_t" in line:
                in_enum = True
            elif in_enum and "}" in line:
                break
            elif in_enum:
                line = line.strip()
                if line and not line.startswith('//') and '=' in line:
                    domain = line.split('=')[0].strip()
                    if domain != "_count":
                        domains.append(domain)

    cc_file = os.path.join(root_dir, "src/i18n/json_translator.cc")
    
    out = []
    out.append('#include "error_system/i18n/json_translator.h"')
    out.append('#include "error_system/core/error_code.h"')
    out.append('#include "error_system/core/error_level.h"')
    out.append('#include "error_system/domain/system_domain.h"')
    out.append('#include <sstream>')
    
    for d in domains:
        out.append(f'#include "error_system/traits/subsystem/{d}_subsystem_traits.h"')
        out.append(f'#include "error_system/traits/module/{d}_module_traits.h"')

    out.append('\nnamespace error_system::i18n {')
    
    # __initialized_json_dict
    out.append('''    /**
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
''')

    # constructor
    out.append('''    /**
     * @brief 构造函数
     * @param language 翻译器默认语言
     */
    json_translator_t::json_translator_t(language_t language) noexcept : language_(language), json_dict_{} {
        __initialized_json_dict();
    }
''')

    # translate
    out.append('''    /**
     * @brief 翻译错误码
     * @details 将错误码翻译为指定语言的文本
     * @param code 错误码
     * @return std::string 翻译后的文本
     */
    std::string json_translator_t::translate(const core::error_code_t& code) const noexcept {
        if (json_dict_.empty()) {
            return "Translator not initialized or dict missing";
        }

        std::string level_str = core::to_string(code.get_level());
        std::string domain_str = domain::to_string(code.get_system());
        std::string subsys_str = "unknown";
        std::string module_str = "unknown";

        switch (code.get_system()) {''')
    
    for d in domains:
        out.append(f'''            case domain::system_domain_t::{d}:
                subsys_str = traits::subsystem_traits<subsystem::{d}_subsystem_t>::to_string(static_cast<subsystem::{d}_subsystem_t>(code.get_subsys()));
                module_str = traits::module_traits<module::{d}_module_t>::to_string(static_cast<module::{d}_module_t>(code.get_module()));
                break;''')

    out.append('''            default:
                break;
        }

        std::string level_trans = json_dict_.get_value_or("error_level." + level_str, level_str).value();
        std::string domain_trans = json_dict_.get_value_or("domain." + domain_str, domain_str).value();
        std::string subsys_trans = json_dict_.get_value_or("subsystem." + domain_str + "." + subsys_str, subsys_str).value();
        std::string module_trans = json_dict_.get_value_or("module." + domain_str + "." + module_str, module_str).value();

        std::ostringstream oss;
        oss << "[" << level_trans << "] " 
            << domain_trans << " | " 
            << subsys_trans << " | " 
            << module_trans << " "
            << "(Code: " << code.get_number() << ")";

        return oss.str();
    }
''')

    # get_language
    out.append('''    /**
     * @brief 获取翻译器当前设置的语言
     * @return language_t 当前语言
     */
    language_t json_translator_t::get_language() const noexcept {
        return language_;
    }
''')

    # set_language
    out.append('''    /**
     * @brief 设置翻译器当前语言
     * @param lang 目标语言
     */
    void json_translator_t::set_language(language_t lang) noexcept {
        language_ = lang;
        __initialized_json_dict();
    }

}  // namespace error_system::i18n
''')

    with open(cc_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(out))

    print(f"✅ Generated {cc_file} with Doxygen comments!")

if __name__ == "__main__":
    main()
