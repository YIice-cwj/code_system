#include "error_system/config/i18n_config.h"

#include <cstddef>
#include <mutex>

/**
 * @file i18n_config.cc
 * @brief i18n 配置实现
 * @details 承接 i18n_config_t 中含 call_once 与循环的大型函数实现。
 * @author yiice
 * @version 4.0.0
 * @date 2026-07-06
 * @copyright Copyright (c) 2026
 */
namespace error_system::config {

    std::atomic<uint8_t>* i18n_config_t::get_locale_parent_storage_() noexcept {
        static std::atomic<uint8_t> parents[error_system::i18n::LOCALE_COUNT];
        static std::once_flag init_flag;
        std::call_once(init_flag, [] {
            for (size_t i = 0; i < error_system::i18n::LOCALE_COUNT; ++i) {
                parents[i].store(static_cast<uint8_t>(error_system::i18n::LOCALE_PARENT_TABLE[i]));
            }
        });
        return parents;
    }

    void i18n_config_t::reset_all_locale_parents() noexcept {
        for (size_t i = 0; i < error_system::i18n::LOCALE_COUNT; ++i) {
            get_locale_parent_storage_()[i].store(
                static_cast<uint8_t>(error_system::i18n::LOCALE_PARENT_TABLE[i]));
        }
    }

}  // namespace error_system::config
