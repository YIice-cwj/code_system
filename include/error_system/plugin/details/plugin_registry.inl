#pragma once
#include "error_system/plugin/plugin_registry.h"
#include "error_system/utils/bad_alloc_handler.h"

namespace error_system::plugin {

    /**
     * @brief 原子更新插件快照
     * @details 在写锁保护下，基于当前快照创建新副本、修改后原子替换。
     *          旧快照由 shared_ptr 自动管理生命周期。
     *          owned_plugins_ 采用 copy-on-write：先复制到工作副本，modifier 修改工作副本，
     *          全部成功后再 swap 回 owned_plugins_，保证 modifier 抛出 bad_alloc 时
     *          owned_plugins_ 保持不变（强异常安全）。make_shared、owned 复制与 modifier
     *          中的 push_back 可能抛出 std::bad_alloc，捕获后记录日志并返回，保持旧快照不变。
     * @tparam Modifier 修改回调函数类型
     * @param modifier 修改新副本与 owned 工作副本的回调函数
     */
    template <typename Modifier>
    inline void plugin_registry_t::update_snapshot_(Modifier&& modifier) noexcept {
        try {
            std::unique_lock<std::shared_mutex> lock(plugins_mutex_);
            auto old_snapshot = std::atomic_load(&plugins_snapshot_);
            auto new_snapshot_ptr = std::make_shared<plugin_list_t>(*old_snapshot);
            std::vector<shared_plugin_ptr_t> new_owned(owned_plugins_);
            modifier(*new_snapshot_ptr, new_owned);
            owned_plugins_.swap(new_owned);
            std::atomic_store(&plugins_snapshot_,
                              std::static_pointer_cast<const plugin_list_t>(new_snapshot_ptr));
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("plugin_registry", "update_snapshot_");
        }
    }

}  // namespace error_system::plugin
