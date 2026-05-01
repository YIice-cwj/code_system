#pragma once
#include "error_system/i18n/i_translator.h"
#include "error_system/utils/json_utils.h"

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
        language_t language_ = language_t::zh_cn;
        utils::json_dict_t json_dict_;

        private:
        /**
         * @brief 初始化 JSON 字典
         * @details 从 JSON 字典文件中加载 JSON 字典
         * @return bool 是否成功加载 JSON 字典
         */
        bool __initialized_json_dict() noexcept;

        /**
         * @brief 根据子系统枚举值解析对应的子系统名称
         * @details 遍历所有子系统的值范围，找到匹配的子系统并返回其字符串名称，若无匹配则返回 "none"
         * @param subsystem_value 子系统枚举值
         * @return std::string 子系统名称
         */
        std::string __resolve_subsys(uint16_t subsystem_value) const noexcept;

        /**
         * @brief 根据模块枚举值解析对应的模块名称
         * @details 遍历所有模块的值范围，找到匹配的模块并返回其字符串名称，若无匹配则返回 "none"
         * @param module_value 模块枚举值
         * @return std::string 模块名称
         */
        std::string __resolve_module(uint16_t module_value) const noexcept;

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
         * @brief 构造函数
         * @param language 翻译器默认语言
         */
        json_translator_t(language_t language) noexcept;

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
