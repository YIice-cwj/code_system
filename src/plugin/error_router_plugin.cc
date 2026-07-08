#include "error_system/plugin/error_router_plugin.h"

/**
 * @file error_router_plugin.cc
 * @brief 错误路由插件实现，按码/域/模块组分发
 * @details 实现按错误码、模块组 ID、系统域三级匹配的处理函数分发，匹配优先级为 码 > 模块组 > 域。
 *          基于 std::call_once 实现线程安全的单例初始化。
 *          Lean 路径（on_code）路由到 code_handler_t，与 Full 路径独立。
 * @author yiice
 * @version 4.4.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <mutex>

#include "error_system/utils/bad_alloc_handler.h"
#include "error_system/utils/log.h"

namespace error_system::plugin {

    namespace {
        /**
         * @brief 通用处理函数注册（Full/Lean 路径共用）
         * @details 加锁写入指定 map，空 handler 跳过，bad_alloc 上报日志
         * @tparam Key map 键类型
         * @tparam Value map 值类型（error_handler_t / code_handler_t）
         * @param mutex 互斥量
         * @param map 目标 map
         * @param key 注册键
         * @param handler 处理函数（按值传入以接管所有权）
         * @param fn_name 调用方函数名，用于日志上报
         */
        template <typename Key, typename Value>
        void register_handler_impl_(std::shared_mutex& mutex, std::unordered_map<Key, Value>& map,
                                    const Key& key, Value handler, const char* fn_name) noexcept {
            if (!handler) {
                return;
            }
            try {
                std::unique_lock<std::shared_mutex> lock(mutex);
                map[key] = std::move(handler);
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("error_router_plugin", fn_name);
            }
        }

        /**
         * @brief 通用处理函数注销（Full/Lean 路径共用）
         * @details 加锁擦除指定 map 的键
         * @tparam Key map 键类型
         * @tparam Value map 值类型
         * @param mutex 互斥量
         * @param map 目标 map
         * @param key 注销键
         * @param fn_name 调用方函数名，用于日志上报
         */
        template <typename Key, typename Value>
        void unregister_handler_impl_(std::shared_mutex& mutex, std::unordered_map<Key, Value>& map,
                                      const Key& key, const char* fn_name) noexcept {
            try {
                std::unique_lock<std::shared_mutex> lock(mutex);
                map.erase(key);
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("error_router_plugin", fn_name);
            }
        }
    }  // namespace

    /**
     * @brief 错误事件回调
     * @details 当一个 error_context_t 被创建时触发，实现此方法进行日志/统计等处理
     * @param context 错误上下文（只读）
     */
    void error_router_plugin_t::on_error(const core::error_context_t& context) noexcept {
        error_handler_t handler;

        try {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (auto it_specific = specific_handlers_.find(context.get_code().get_code()); it_specific != specific_handlers_.end()) {
                handler = it_specific->second;
            } else if (auto it_module = module_group_handlers_.find(context.get_code().get_module_group_id());
                       it_module != module_group_handlers_.end()) {
                handler = it_module->second;
            } else if (auto it_domain = domain_handlers_.find(context.get_code().get_system());
                       it_domain != domain_handlers_.end()) {
                handler = it_domain->second;
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[error_router_plugin] on_error: handler copy std::bad_alloc");
            return;
        }

        if (handler) {
            try {
                handler(context);
            } catch (const std::exception& e) {
                LOG_ERROR("[error_router_plugin] handler exception: {}", e.what());
            }
        }
    }

    /**
     * @brief 错误码通知回调（Lean 路径）
     * @details 按 码 > 模块组 > 域 三级优先级匹配 code_handler_t 并调用。
     *          handler 拷贝在锁内完成，调用在锁外执行，避免持锁回调死锁。
     */
    void error_router_plugin_t::on_code(core::error_code_t code) noexcept {
        code_handler_t handler;

        try {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (auto it_specific = code_specific_handlers_.find(code.get_code()); it_specific != code_specific_handlers_.end()) {
                handler = it_specific->second;
            } else if (auto it_module = code_module_group_handlers_.find(code.get_module_group_id());
                       it_module != code_module_group_handlers_.end()) {
                handler = it_module->second;
            } else if (auto it_domain = code_domain_handlers_.find(code.get_system());
                       it_domain != code_domain_handlers_.end()) {
                handler = it_domain->second;
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[error_router_plugin] on_code: handler copy std::bad_alloc");
            return;
        }

        if (handler) {
            try {
                handler(code);
            } catch (const std::exception& e) {
                LOG_ERROR("[error_router_plugin] code handler exception: {}", e.what());
            }
        }
    }

    /**
     * @brief 按错误码注册处理函数
     * @param code 错误码
     * @param handler 处理函数
     */
    void error_router_plugin_t::register_handler_by_code(const core::error_code_t& code,
                                                       error_handler_t handler) noexcept {
        register_handler_impl_(mutex_, specific_handlers_, code.get_code(), std::move(handler),
                               "register_handler_by_code");
    }

    /**
     * @brief 按模块组 ID 注册处理函数
     * @param module_group_id 模块组 ID
     * @param handler 处理函数
     */
    void error_router_plugin_t::register_handler_by_module_group_id(core::module_group_id_t module_group_id,
                                                                    error_handler_t handler) noexcept {
        register_handler_impl_(mutex_, module_group_handlers_, module_group_id, std::move(handler),
                               "register_handler_by_module_group_id");
    }

    /**
     * @brief 按系统域注册处理函数
     * @param domain 系统域
     * @param handler 处理函数
     */
    void error_router_plugin_t::register_handler_by_domain(domain::system_domain_t domain,
                                                           error_handler_t handler) noexcept {
        register_handler_impl_(mutex_, domain_handlers_, domain, std::move(handler),
                               "register_handler_by_domain");
    }

    /**
     * @brief 移除按错误码注册的处理函数
     * @param code 错误码
     */
    void error_router_plugin_t::unregister_handler_by_code(const core::error_code_t& code) noexcept {
        unregister_handler_impl_(mutex_, specific_handlers_, code.get_code(), "unregister_handler_by_code");
    }

    /**
     * @brief 移除按模块组 ID 注册的处理函数
     * @param module_group_id 模块组 ID
     */
    void
    error_router_plugin_t::unregister_handler_by_module_group_id(core::module_group_id_t module_group_id) noexcept {
        unregister_handler_impl_(mutex_, module_group_handlers_, module_group_id,
                                 "unregister_handler_by_module_group_id");
    }

    /**
     * @brief 移除按系统域注册的处理函数
     * @param domain 系统域
     */
    void error_router_plugin_t::unregister_handler_by_domain(domain::system_domain_t domain) noexcept {
        unregister_handler_impl_(mutex_, domain_handlers_, domain, "unregister_handler_by_domain");
    }

    void error_router_plugin_t::register_code_handler_by_code(const core::error_code_t& code,
                                                               code_handler_t handler) noexcept {
        register_handler_impl_(mutex_, code_specific_handlers_, code.get_code(), std::move(handler),
                               "register_code_handler_by_code");
    }

    void error_router_plugin_t::register_code_handler_by_module_group_id(core::module_group_id_t module_group_id,
                                                                          code_handler_t handler) noexcept {
        register_handler_impl_(mutex_, code_module_group_handlers_, module_group_id, std::move(handler),
                               "register_code_handler_by_module_group_id");
    }

    void error_router_plugin_t::register_code_handler_by_domain(domain::system_domain_t domain,
                                                                  code_handler_t handler) noexcept {
        register_handler_impl_(mutex_, code_domain_handlers_, domain, std::move(handler),
                               "register_code_handler_by_domain");
    }

    void error_router_plugin_t::unregister_code_handler_by_code(const core::error_code_t& code) noexcept {
        unregister_handler_impl_(mutex_, code_specific_handlers_, code.get_code(),
                                 "unregister_code_handler_by_code");
    }

    void error_router_plugin_t::unregister_code_handler_by_module_group_id(core::module_group_id_t module_group_id) noexcept {
        unregister_handler_impl_(mutex_, code_module_group_handlers_, module_group_id,
                                 "unregister_code_handler_by_module_group_id");
    }

    void error_router_plugin_t::unregister_code_handler_by_domain(domain::system_domain_t domain) noexcept {
        unregister_handler_impl_(mutex_, code_domain_handlers_, domain, "unregister_code_handler_by_domain");
    }
}  // namespace error_system::plugin