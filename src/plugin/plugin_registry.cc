#include "error_system/plugin/plugin_registry.h"

/**
 * @file plugin_registry.cc
 * @brief 插件注册表单例实现，支持 sync/async_queue/sync_deferred 三种通知模式
 * @details 提供插件注册、注销、错误事件分发能力。snapshot 基于原子读写实现无锁通知路径；
 *          async_queue 通过通知通道异步排队；sync_deferred 通过线程本地缓冲累积后显式 flush。
 *          context 通知走 on_error，error_code 通知走 on_code（Lean 路径，零堆开销）。
 *          本类实现 core::i_error_notifier_t 接口，并在 instance() 中自注册为默认通知器，
 *          使 core 层经由抽象接口完成通知，解耦 core→plugin 反向依赖。
 * @author yiice
 * @version 4.4.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <algorithm>
#include <mutex>
#include <vector>

#include "error_system/config/feature_flags.h"
#include "error_system/core/i_error_notifier.h"
#include "error_system/utils/bad_alloc_handler.h"
#include "error_system/utils/log.h"

namespace error_system::plugin {

    std::once_flag plugin_registry_t::once_flag_;
    std::atomic<bool> plugin_registry_t::initialized_{false};

    namespace {
        /**
         * @brief 线程本地延迟通知缓冲
         * @details sync_deferred 模式下累积错误上下文与错误码，由调用方显式 flush。
         *          thread_local 避免跨线程同步开销，每个请求处理线程独立缓冲。
         *          缓冲满时丢弃新通知并设置 overflow_dropped 标志。
         *          最大容量统一由 config::feature_flags_t::get_deferred_buffer_max_size() 管理。
         *          code_buffer 与 buffer 共享同一 max_size 预算（按总条目计）。
         */
        struct deferred_buffer_t {
            std::vector<std::shared_ptr<const core::error_context_t>> buffer;
            std::vector<core::error_code_t> code_buffer;
            bool overflow_dropped{false};
            bool flushing{false};
        };

        thread_local deferred_buffer_t tls_deferred_;
    }  // namespace

    /**
     * @brief 构造函数
     * @details 初始化通知通道，将通知回调绑定到本对象的 notify_error 方法。
     * @note noexcept 风险：notification_channel_ 内部 std::function 构造可能抛出 std::bad_alloc，
     *       一旦抛出将触发 std::terminate（构造函数标记为 noexcept）。该风险仅在单例首次构造时存在，
     *       且内存不足场景下系统已无法正常工作，可接受。
     */
    plugin_registry_t::plugin_registry_t() noexcept
        : notification_channel_(
              [this](const core::error_context_t& context) { notify_error(context); },
              [this](core::error_code_t code) { handle_code_notification_(code); }) {}

    /**
     * @brief 插入或替换插件（核心逻辑）
     * @details 同名插件替换旧条目并从 owned 中移除旧指针，新插件追加到 snapshot 与 owned。
     *          find_if + erase(remove_if) 逻辑集中于此，消除 register 路径的重复。
     * @param snapshot 快照副本（写锁内）
     * @param owned 持有所有权列表副本（写锁内）
     * @param new_plugin 新插件 shared_ptr
     */
    void plugin_registry_t::upsert_plugin_(plugin_list_t& snapshot,
                                           std::vector<shared_plugin_ptr_t>& owned,
                                           shared_plugin_ptr_t new_plugin) noexcept {
        auto name = new_plugin->name();
        auto it = std::find_if(snapshot.begin(), snapshot.end(),
            [&name](const shared_plugin_ptr_t& sp) { return sp->name() == name; });
        if (it != snapshot.end()) {
            auto* old_ptr = it->get();
            owned.erase(std::remove_if(owned.begin(), owned.end(),
                            [old_ptr](const shared_plugin_ptr_t& sp) { return sp.get() == old_ptr; }),
                        owned.end());
            *it = new_plugin;
        } else {
            snapshot.push_back(new_plugin);
        }
        owned.push_back(std::move(new_plugin));
    }

    /**
     * @brief 注册插件（转移所有权）
     * @details 将插件包装为 shared_ptr 并委托 upsert_plugin_。
     * @param plugin 待注册的插件 unique_ptr，空指针将被忽略
     */
    void plugin_registry_t::register_plugin(std::unique_ptr<i_error_plugin_t> plugin) noexcept {
        if (!plugin) {
            return;
        }
        auto shared_plugin = std::shared_ptr<i_error_plugin_t>(std::move(plugin));
        update_snapshot_([this, &shared_plugin](plugin_list_t& snapshot,
                                          std::vector<shared_plugin_ptr_t>& owned) {
            upsert_plugin_(snapshot, owned, std::move(shared_plugin));
        });
    }

    /**
     * @brief 注册插件（非持有引用）
     * @details 构造空删除器 shared_ptr 并委托 upsert_plugin_。
     * @param plugin 插件引用
     */
    void plugin_registry_t::register_plugin_ref(i_error_plugin_t& plugin) noexcept {
        auto non_owning = std::shared_ptr<i_error_plugin_t>(&plugin, [](i_error_plugin_t*){});
        update_snapshot_([this, &non_owning](plugin_list_t& snapshot,
                                       std::vector<shared_plugin_ptr_t>& owned) {
            upsert_plugin_(snapshot, owned, non_owning);
        });
    }

    void plugin_registry_t::unregister_plugin(std::string_view name) noexcept {
        update_snapshot_([name](plugin_list_t& snapshot, std::vector<shared_plugin_ptr_t>& owned) {
            auto it = std::find_if(snapshot.begin(), snapshot.end(),
                [name](const shared_plugin_ptr_t& plugin) { return plugin->name() == name; });
            if (it == snapshot.end()) {
                return;
            }
            auto* old_ptr = it->get();
            snapshot.erase(it);
            owned.erase(std::remove_if(owned.begin(), owned.end(),
                            [old_ptr](const shared_plugin_ptr_t& plugin) { return plugin.get() == old_ptr; }),
                        owned.end());
        });
    }

    namespace {

        /**
         * @brief 向所有插件分发单个错误上下文
         * @details 遍历插件快照，按 min_level 过滤后调用 on_error。
         *          on_error 为 noexcept override，插件实现必须保证不抛异常，否则 std::terminate。
         *          notify_error / flush_deferred_notifications 统一委托本函数，
         *          消除插件遍历与等级过滤的重复代码。
         */
        void dispatch_to_plugins(const core::error_context_t& context,
                                 const std::vector<std::shared_ptr<i_error_plugin_t>>& plugins) noexcept {
            for (const auto& plugin : plugins) {
                if (context.get_code().get_level() < plugin->min_level()) {
                    continue;
                }
                plugin->on_error(context);
            }
        }

        /**
         * @brief 向所有插件分发单个错误码（Lean 路径）
         * @details 遍历插件快照，按 min_level 过滤后调用 on_code。
         *          不构造 error_context_t，零堆开销。
         *          notify_code_error_ / flush_deferred_notifications 统一委托本函数。
         */
        void dispatch_code_to_plugins(core::error_code_t code,
                                      const std::vector<std::shared_ptr<i_error_plugin_t>>& plugins) noexcept {
            for (const auto& plugin : plugins) {
                if (code.get_level() < plugin->min_level()) {
                    continue;
                }
                plugin->on_code(code);
            }
        }

        /**
         * @brief 通用延迟入队实现（context/code 路径共用）
         * @details flushing 状态跳过；总量超限置 overflow_dropped；
         *          push 抛 bad_alloc 上报日志并置溢出标志。
         * @tparam PushFn 入队可调用对象
         * @param push_fn 实际入队操作
         * @param fn_name 调用方函数名，用于日志上报
         */
        template <typename PushFn>
        void enqueue_deferred_impl_(PushFn push_fn, const char* fn_name) noexcept {
            if (tls_deferred_.flushing) {
                return;
            }
            const size_t max_size = config::feature_flags_t::get_deferred_buffer_max_size();
            const size_t total = tls_deferred_.buffer.size() + tls_deferred_.code_buffer.size();
            if (max_size > 0 && total >= max_size) {
                tls_deferred_.overflow_dropped = true;
                return;
            }
            try {
                push_fn();
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("plugin_registry", fn_name);
                tls_deferred_.overflow_dropped = true;
            }
        }

    }  // namespace

    void plugin_registry_t::notify_error(const core::error_context_t& context) noexcept {
        dispatch_to_plugins(context, *load_snapshot_());
    }

    void plugin_registry_t::notify_code_error_(core::error_code_t code) noexcept {
        dispatch_code_to_plugins(code, *load_snapshot_());
    }

    void plugin_registry_t::notify(const core::error_context_t& context) noexcept {
        const auto mode = config::feature_flags_t::get_notify_mode();
        if (mode == config::feature_flags_t::notify_mode_t::async_queue) {
            enqueue_notification(context);
        } else if (mode == config::feature_flags_t::notify_mode_t::sync_deferred) {
            enqueue_deferred_notification(context);
        } else {
            notify_error(context);
        }
    }

    void plugin_registry_t::notify(core::error_code_t code) noexcept {
        const auto mode = config::feature_flags_t::get_notify_mode();
        if (mode == config::feature_flags_t::notify_mode_t::async_queue) {
            notification_channel_.enqueue_code(code);
        } else if (mode == config::feature_flags_t::notify_mode_t::sync_deferred) {
            enqueue_deferred_code(code);
        } else {
            notify_code_error_(code);
        }
    }

    void plugin_registry_t::handle_code_notification_(core::error_code_t code) noexcept {
        notify_code_error_(code);
    }

    void plugin_registry_t::enqueue_deferred_notification(const core::error_context_t& context) noexcept {
        enqueue_deferred_impl_(
            [&] {
                tls_deferred_.buffer.push_back(
                    std::make_shared<const core::error_context_t>(context.clone()));
            },
            "enqueue_deferred_notification");
    }

    void plugin_registry_t::enqueue_deferred_code(core::error_code_t code) noexcept {
        enqueue_deferred_impl_(
            [&] { tls_deferred_.code_buffer.push_back(code); }, "enqueue_deferred_code");
    }

    void plugin_registry_t::flush_deferred_notifications() noexcept {
        if (tls_deferred_.flushing) {
            return;
        }
        if (tls_deferred_.buffer.empty() && tls_deferred_.code_buffer.empty()) {
            tls_deferred_.overflow_dropped = false;
            return;
        }
        tls_deferred_.flushing = true;
        auto snapshot = load_snapshot_();
        for (const auto& deferred_context : tls_deferred_.buffer) {
            if (deferred_context) {
                dispatch_to_plugins(*deferred_context, *snapshot);
            }
        }
        for (const auto& deferred_code : tls_deferred_.code_buffer) {
            dispatch_code_to_plugins(deferred_code, *snapshot);
        }
        tls_deferred_.buffer.clear();
        tls_deferred_.code_buffer.clear();
        tls_deferred_.overflow_dropped = false;
        tls_deferred_.flushing = false;
    }

    /**
     * @brief 获取当前线程待刷新的延迟通知数量
     * @return size_t 缓冲中的通知数量（context + code）
     */
    size_t plugin_registry_t::pending_deferred_notifications() const noexcept {
        return tls_deferred_.buffer.size() + tls_deferred_.code_buffer.size();
    }

    size_t plugin_registry_t::clear_deferred_notifications() noexcept {
        const size_t dropped = tls_deferred_.buffer.size() + tls_deferred_.code_buffer.size();
        tls_deferred_.buffer.clear();
        tls_deferred_.code_buffer.clear();
        tls_deferred_.overflow_dropped = false;
        return dropped;
    }

    bool plugin_registry_t::deferred_buffer_overflowed() const noexcept {
        return tls_deferred_.overflow_dropped;
    }

    /**
     * @brief 清空所有已注册插件
     */
    void plugin_registry_t::clear() noexcept {
        update_snapshot_([](plugin_list_t& snapshot, std::vector<shared_plugin_ptr_t>& owned) {
            snapshot.clear();
            owned.clear();
        });
    }

    /**
     * @brief 获取单例实例
     * @details 使用 std::call_once + 函数局部静态保证线程安全的单例初始化。
     *          初始化完成后自注册为默认错误通知器（仅在未设置自定义 notifier 时）。
     *          若已存在自定义通知器，输出提示信息，避免用户误以为插件注册表已被使用。
     * @return 单例引用
     */
    plugin_registry_t& plugin_registry_t::instance() noexcept {
        static plugin_registry_t* instance_ptr = nullptr;
        std::call_once(once_flag_, [] {
            static plugin_registry_t instance;
            instance_ptr = &instance;
            initialized_.store(true, std::memory_order_release);
            auto* existing_notifier = core::i_error_notifier_t::get_current();
            if (existing_notifier == nullptr) {
                core::i_error_notifier_t::set_current(instance_ptr);
            } else if (existing_notifier != instance_ptr) {
                LOG_INFO("[plugin_registry] instance: custom error notifier already set; "
                         "plugin_registry_t will not receive error notifications.");
            }
        });
        return *instance_ptr;
    }

}  // namespace error_system::plugin
