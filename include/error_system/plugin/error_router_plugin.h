#pragma once
#include <functional>
// IWYU pragma: begin_exports
#include <mutex>
// IWYU pragma: end_exports
#include <shared_mutex>
#include <unordered_map>

#include "error_system/core/error_code.h"
#include "error_system/plugin/i_error_plugin.h"
#include "error_system/utils/singleton.h"

/**
 * @file error_router_plugin.h
 * @brief 错误路由插件
 * @details 将错误事件按 码 > 模块组 > 域 三级优先级路由到已注册的处理函数，
 *          例如：日志记录、统计分析、告警通知等。
 *
 *          双路径设计：
 *          - Full 路径（on_error）：路由到 error_handler_t，接收完整 error_context_t
 *          - Lean 路径（on_code）：路由到 code_handler_t，仅接收 error_code_t
 *          两路径 handler 独立注册，按 code/module_group/domain 三级优先级匹配，
 *          互不影响。Lean 路径下若未注册 code_handler，则静默跳过。
 * @author yiice
 * @version 4.4.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::plugin {

    /**
     * @brief 完整上下文处理函数类型
     * @param error_context 错误上下文
     */
    using error_handler_t = std::function<void(const core::error_context_t&)>;

    /**
     * @brief 纯错误码处理函数类型（Lean 路径）
     * @param code 错误码
     */
    using code_handler_t = std::function<void(core::error_code_t)>;

    /**
     * @brief 错误路由插件
     * @details 错误路由插件负责将错误事件路由到对应的处理函数
     */
    class error_router_plugin_t : public i_error_plugin_t,
                                  public utils::singleton_t<error_router_plugin_t> {
        friend class utils::singleton_t<error_router_plugin_t>;
    private:
        std::unordered_map<core::code_t, error_handler_t> specific_handlers_{};

        std::unordered_map<core::module_group_id_t, error_handler_t> module_group_handlers_{};

        std::unordered_map<domain::system_domain_t, error_handler_t> domain_handlers_{};

        std::unordered_map<core::code_t, code_handler_t> code_specific_handlers_{};

        std::unordered_map<core::module_group_id_t, code_handler_t> code_module_group_handlers_{};

        std::unordered_map<domain::system_domain_t, code_handler_t> code_domain_handlers_{};

        std::string name_{"global_error_router"};

        mutable std::shared_mutex mutex_;

        error_router_plugin_t() = default;

    public:
        ~error_router_plugin_t() noexcept override = default;

        /**
         * @brief 错误事件回调
         * @details 当一个 error_context_t 被创建时触发，实现此方法进行日志/统计等处理
         * @param context 错误上下文（只读）
         */
        void on_error(const core::error_context_t& context) noexcept override;

        /**
         * @brief 错误码通知回调（Lean 路径）
         * @details 按 码 > 模块组 > 域 三级优先级匹配 code_handler_t。
         *          若未注册任何 code_handler，则静默跳过。
         * @param code 错误码
         */
        void on_code(core::error_code_t code) noexcept override;

        /**
         * @brief 按错误码注册处理函数
         * @param code 错误码
         * @param handler 处理函数
         */
        void register_handler_by_code(const core::error_code_t& code, error_handler_t handler) noexcept;

        /**
         * @brief 按模块组 ID 注册处理函数
         * @param module_group_id 模块组 ID
         * @param handler 处理函数
         */
        void register_handler_by_module_group_id(core::module_group_id_t module_group_id,
                                                 error_handler_t handler) noexcept;

        /**
         * @brief 按系统域注册处理函数
         * @param domain 系统域
         * @param handler 处理函数
         */
        void register_handler_by_domain(domain::system_domain_t domain, error_handler_t handler) noexcept;

        /**
         * @brief 移除按错误码注册的处理函数
         * @param code 错误码
         */
        void unregister_handler_by_code(const core::error_code_t& code) noexcept;

        /**
         * @brief 移除按模块组 ID 注册的处理函数
         * @param module_group_id 模块组 ID
         */
        void unregister_handler_by_module_group_id(core::module_group_id_t module_group_id) noexcept;

        /**
         * @brief 移除按系统域注册的处理函数
         * @param domain 系统域
         */
        void unregister_handler_by_domain(domain::system_domain_t domain) noexcept;

        /**
         * @brief 按错误码注册 Lean 路径处理函数
         * @param code 错误码
         * @param handler 纯错误码处理函数
         */
        void register_code_handler_by_code(const core::error_code_t& code, code_handler_t handler) noexcept;

        /**
         * @brief 按模块组 ID 注册 Lean 路径处理函数
         * @param module_group_id 模块组 ID
         * @param handler 纯错误码处理函数
         */
        void register_code_handler_by_module_group_id(core::module_group_id_t module_group_id,
                                                      code_handler_t handler) noexcept;

        /**
         * @brief 按系统域注册 Lean 路径处理函数
         * @param domain 系统域
         * @param handler 纯错误码处理函数
         */
        void register_code_handler_by_domain(domain::system_domain_t domain, code_handler_t handler) noexcept;

        /**
         * @brief 移除按错误码注册的 Lean 路径处理函数
         * @param code 错误码
         */
        void unregister_code_handler_by_code(const core::error_code_t& code) noexcept;

        /**
         * @brief 移除按模块组 ID 注册的 Lean 路径处理函数
         * @param module_group_id 模块组 ID
         */
        void unregister_code_handler_by_module_group_id(core::module_group_id_t module_group_id) noexcept;

        /**
         * @brief 移除按系统域注册的 Lean 路径处理函数
         * @param domain 系统域
         */
        void unregister_code_handler_by_domain(domain::system_domain_t domain) noexcept;

        /**
         * @brief 获取插件名称
         * @details 用于标识插件，注册时若名称重复则替换旧插件
         * @return std::string_view 插件名称
         */
        [[nodiscard]] std::string_view name() const noexcept override {
            return name_;
        }
    };
}  // namespace error_system::plugin