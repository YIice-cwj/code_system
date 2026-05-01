#include "error_system/plugin/plugin_registry.h"
#include <algorithm>

namespace error_system::plugin {

    /**
     * @brief 获取单例实例
     */
    plugin_registry_t& plugin_registry_t::instance() noexcept {
        static plugin_registry_t instance;
        return instance;
    }

    /**
     * @brief 注册插件
     * @details 若已存在同名插件，替换旧插件
     */
    void plugin_registry_t::register_plugin(i_error_plugin_t* plugin) noexcept {
        if (!plugin) return;

        auto it = std::find_if(plugins_.begin(), plugins_.end(),
            [&](const i_error_plugin_t* p) { return p->name() == plugin->name(); });

        if (it != plugins_.end()) {
            *it = plugin;  // 同名替换
        } else {
            plugins_.push_back(plugin);
        }
    }

    /**
     * @brief 注销插件
     */
    void plugin_registry_t::unregister_plugin(std::string_view name) noexcept {
        plugins_.erase(
            std::remove_if(plugins_.begin(), plugins_.end(),
                [&](const i_error_plugin_t* p) { return p->name() == name; }),
            plugins_.end()
        );
    }

    /**
     * @brief 通知所有插件发生了错误事件
     */
    void plugin_registry_t::notify_error(const core::error_context_t& context) noexcept {
        for (auto* plugin : plugins_) {
            plugin->on_error(context);
        }
    }

    /**
     * @brief 获取已注册插件数量
     */
    size_t plugin_registry_t::size() const noexcept {
        return plugins_.size();
    }

    /**
     * @brief 判断是否有已注册的插件
     */
    bool plugin_registry_t::empty() const noexcept {
        return plugins_.empty();
    }

}  // namespace error_system::plugin
