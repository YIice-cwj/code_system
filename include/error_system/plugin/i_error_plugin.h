#pragma once
#include <string_view>

#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"

/**
 * @file i_error_plugin.h
 * @brief 错误系统插件接口
 * @details 定义错误系统插件的基础接口，插件可接收错误事件并进行处理，
 *          例如：日志记录、统计分析、告警通知等。
 *          插件可同时实现 on_error（完整上下文）与 on_code（仅错误码）两种回调，
 *          后者用于 Lean 通知路径，避免构造完整 error_context_t。
 * @author yiice
 * @version 4.4.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::plugin {

    /**
     * @brief 错误系统插件接口
     * @details 继承此接口实现自定义插件，注册到 plugin_registry_t 后，
     *          将在每次错误上下文创建时自动收到 on_error() 回调。
     *          通过 min_level() 可过滤低于指定级别的错误事件。
     *          Lean 通知路径下仅传递 error_code_t，插件可按需 override on_code()。
     */
    class i_error_plugin_t {
    public:
        virtual ~i_error_plugin_t() noexcept = default;

        /**
         * @brief 获取插件名称
         * @details 用于标识插件，注册时若名称重复则替换旧插件
         * @return std::string_view 插件名称
         */
        [[nodiscard]] virtual std::string_view name() const noexcept = 0;

        /**
         * @brief 获取插件关注的最低错误级别
         * @details 仅接收级别 >= 此值的错误事件，默认返回 debug（接收所有级别）
         * @return error_level_t 最低错误级别
         */
        [[nodiscard]] virtual core::error_level_t min_level() const noexcept { return core::error_level_t::debug; }

        /**
         * @brief 错误事件回调
         * @details 当一个 error_context_t 被创建时触发，实现此方法进行日志/统计等处理
         * @param context 错误上下文（只读）
         */
        virtual void on_error(const core::error_context_t& context) noexcept = 0;

        /**
         * @brief 错误码通知回调（Lean 路径）
         * @details Lean 通知路径下触发，仅传递 error_code_t，避免构造完整 error_context_t。
         *          默认空实现，需要处理 code-only 通知的插件自行 override。
         *          插件可凭 code 从 error_registry 反查静态元数据（name/description/level）。
         * @param code 错误码
         */
        virtual void on_code(core::error_code_t code) noexcept {
            (void)code;
        }
    };

}  // namespace error_system::plugin
