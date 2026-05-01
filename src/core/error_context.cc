#include "error_system/core/error_context.h"
#include "error_system/plugin/plugin_registry.h"

namespace error_system::core {


    /**
     * @brief 通知所有已注册插件
     * @details 实现在 plugin_registry_t::notify_error()
     */
    void __notify_plugins(const error_context_t& context) noexcept {
        plugin::plugin_registry_t::instance().notify_error(context);
    }

}  // namespace error_system::core