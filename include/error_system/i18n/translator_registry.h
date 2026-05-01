#pragma once
#include "error_system/i18n/i_translator.h"

/**
 * @file translator_registry.h
 * @brief 全局翻译器注册表
 * @details 提供全局翻译器的注册与获取功能，用于在整个系统中共享同一个翻译器实例
 * @author yiice
 * @version 1.0.0
 * @date 2026-05-01
 * @copyright Copyright (c) 2026
 */
namespace error_system::i18n {

    /**
     * @brief 全局翻译器注册表
     * @details 单例模式，线程安全（由调用方保证初始化时机）
     *          注意：不持有翻译器的所有权，调用方需保证其生命周期
     */
    class translator_registry_t {
        private:
        mutable i_translator_t* translator_{nullptr};

        private:
        translator_registry_t() = default;

        ~translator_registry_t() = default;

        translator_registry_t(const translator_registry_t&) = delete;

        translator_registry_t& operator=(const translator_registry_t&) = delete;

        translator_registry_t(translator_registry_t&&) = delete;

        translator_registry_t& operator=(translator_registry_t&&) = delete;

        public:
        /**
         * @brief 获取单例实例
         * @return translator_registry_t& 单例引用
         */
        static translator_registry_t& instance() noexcept;

        /**
         * @brief 注册全局翻译器
         * @details 不持有所有权，调用方需保证翻译器在整个使用期间有效
         * @param translator 翻译器指针，传入 nullptr 可清除注册
         */
        void set(i_translator_t* translator) noexcept;

        /**
         * @brief 获取当前全局翻译器
         * @details 若未显式注册，首次调用时自动创建并注册默认的中文翻译器
         * @return i_translator_t* 全局翻译器指针（永不为 nullptr）
         */
        i_translator_t* get() const noexcept;

        /**
         * @brief 判断是否已由用户显式注册了翻译器
         * @details 返回 false 不代表 get() 为空，仅表示当前使用的是默认中文翻译器
         * @return bool 是否已有显式注册
         */
        bool has_translator() const noexcept;
    };

}  // namespace error_system::i18n
