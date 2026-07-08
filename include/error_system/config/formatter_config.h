#pragma once
#include <cstdio>
#include <functional>
#include <mutex>
#include <new>
#include <shared_mutex>
#include <string>
#include <utility>

#include "error_system/utils/bad_alloc_handler.h"

namespace error_system::core {
    class error_context_t;
}

namespace error_system::i18n {
    class i_subsystem_module_resolver_t;
}

/**
 * @file formatter_config.h
 * @brief 格式化器与解析器配置类
 * @details 单一职责：管理错误上下文的自定义格式化函数与子系统/模块名称解析器。
 *          提供线程安全的 set/get 接口，通过 shared_mutex 实现读多写少场景下的并发访问。
 * @author yiice
 * @version 3.1.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::config {

    /**
     * @brief 自定义格式化回调函数类型
     * @details 接受 error_context_t 引用，返回格式化后的字符串
     */
    using formatter_callback_t = std::function<std::string(const core::error_context_t&)>;

    /**
     * @brief 格式化器与解析器配置类
     * @details 管理错误上下文的自定义格式化函数与文本序列化所需的子系统/模块名称解析器。
     *          类仅包含静态成员，禁止实例化。
     */
    class formatter_config_t {
    private:
        /**
         * @brief 格式化函数专用共享锁
         * @details 保护自定义格式化函数与解析器的互斥锁
         * @return std::shared_mutex& 共享锁引用
         */
        static std::shared_mutex& get_formatter_mutex_() noexcept;

        /**
         * @brief 自定义格式化函数存储
         * @details 使用 shared_mutex 保护并发读写
         * @return formatter_callback_t& 自定义格式化函数引用
         */
        static formatter_callback_t& get_custom_formatter_() noexcept;

        /**
         * @brief 子系统/模块名称解析器存储
         * @details 文本序列化使用，nullptr 表示使用 i18n 默认解析器
         * @return const i18n::i_subsystem_module_resolver_t*& 解析器指针引用
         */
        static const i18n::i_subsystem_module_resolver_t*& get_subsystem_module_resolver_() noexcept;

    public:
        formatter_config_t() = delete;
        ~formatter_config_t() = delete;
        formatter_config_t(const formatter_config_t&) = delete;
        formatter_config_t& operator=(const formatter_config_t&) = delete;
        formatter_config_t(formatter_config_t&&) = delete;
        formatter_config_t& operator=(formatter_config_t&&) = delete;

        /**
         * @brief 设置自定义格式化函数
         * @param formatter 自定义格式化函数
         */
        static void set_custom_formatter(formatter_callback_t formatter) noexcept;

        /**
         * @brief 获取自定义格式化函数副本
         * @details 返回格式化函数的线程安全拷贝，调用方无需持有锁即可安全调用。
         *          内存不足时返回空回调（nullptr）。
         * @return formatter_callback_t 格式化函数副本
         */
        [[nodiscard]] static formatter_callback_t get_custom_formatter() noexcept;

        /**
         * @brief 设置文本序列化使用的子系统/模块名称解析器
         * @param resolver 解析器接口指针，传入 nullptr 恢复默认解析器
         */
        static void set_subsystem_module_resolver(
            const i18n::i_subsystem_module_resolver_t* resolver) noexcept;

        /**
         * @brief 获取当前子系统/模块名称解析器
         * @return 解析器指针，未设置时返回 nullptr
         */
        [[nodiscard]] static const i18n::i_subsystem_module_resolver_t* get_subsystem_module_resolver() noexcept;
    };

    inline std::shared_mutex& formatter_config_t::get_formatter_mutex_() noexcept {
        static std::shared_mutex mutex;
        return mutex;
    }

    inline formatter_callback_t& formatter_config_t::get_custom_formatter_() noexcept {
        static formatter_callback_t formatter{nullptr};
        return formatter;
    }

    inline const i18n::i_subsystem_module_resolver_t*&
    formatter_config_t::get_subsystem_module_resolver_() noexcept {
        static const i18n::i_subsystem_module_resolver_t* resolver{nullptr};
        return resolver;
    }

    inline void formatter_config_t::set_custom_formatter(formatter_callback_t formatter) noexcept {
        try {
            std::unique_lock<std::shared_mutex> lock(get_formatter_mutex_());
            get_custom_formatter_() = std::move(formatter);
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("formatter_config", "set_custom_formatter");
        }
    }

    inline formatter_callback_t formatter_config_t::get_custom_formatter() noexcept {
        try {
            std::shared_lock<std::shared_mutex> lock(get_formatter_mutex_());
            return get_custom_formatter_();
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("formatter_config", "get_custom_formatter");
            return formatter_callback_t{nullptr};
        }
    }

    inline void formatter_config_t::set_subsystem_module_resolver(
        const i18n::i_subsystem_module_resolver_t* resolver) noexcept {
        std::unique_lock<std::shared_mutex> lock(get_formatter_mutex_());
        get_subsystem_module_resolver_() = resolver;
    }

    inline const i18n::i_subsystem_module_resolver_t*
    formatter_config_t::get_subsystem_module_resolver() noexcept {
        std::shared_lock<std::shared_mutex> lock(get_formatter_mutex_());
        return get_subsystem_module_resolver_();
    }

}  // namespace error_system::config
