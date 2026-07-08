#include "error_system/core/i_error_notifier.h"

/**
 * @file i_error_notifier.cc
 * @brief 错误通知器接口的全局状态实现
 * @details 仅实现 i_error_notifier_t 的静态成员 current_ 及其访问方法、try_notify 封装。
 *          通知逻辑由具体实现类（如 plugin_registry_t）提供。
 * @author yiice
 * @version 4.2.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    i_error_notifier_t* i_error_notifier_t::current_{nullptr};

    void i_error_notifier_t::set_current(i_error_notifier_t* notifier) noexcept {
        current_ = notifier;
    }

    i_error_notifier_t* i_error_notifier_t::get_current() noexcept {
        return current_;
    }

    void i_error_notifier_t::try_notify(const error_context_t& context) noexcept {
        auto* notifier = get_current();
        if (notifier != nullptr) {
            notifier->notify(context);
        }
    }

    void i_error_notifier_t::try_notify(error_code_t code) noexcept {
        auto* notifier = get_current();
        if (notifier != nullptr) {
            notifier->notify(code);
        }
    }

}  // namespace error_system::core
