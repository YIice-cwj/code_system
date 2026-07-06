#include "error_system/config/formatter_config.h"

#include <cstdio>
#include <new>
#include <shared_mutex>
#include <utility>

/**
 * @file formatter_config.cc
 * @brief 自定义格式化器配置实现
 * @details 承接 formatter_config_t 中含锁与异常处理的大型函数实现。
 * @author yiice
 * @version 4.0.0
 * @date 2026-07-06
 * @copyright Copyright (c) 2026
 */
namespace error_system::config {

    void formatter_config_t::set_custom_formatter(formatter_callback_t formatter) noexcept {
        try {
            std::unique_lock<std::shared_mutex> lock(get_formatter_mutex_());
            get_custom_formatter_() = std::move(formatter);
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[formatter_config] set_custom_formatter: std::bad_alloc\n");
        }
    }

    formatter_callback_t formatter_config_t::get_custom_formatter() noexcept {
        try {
            std::shared_lock<std::shared_mutex> lock(get_formatter_mutex_());
            return get_custom_formatter_();
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[formatter_config] get_custom_formatter: std::bad_alloc\n");
            return formatter_callback_t{nullptr};
        }
    }

}  // namespace error_system::config
