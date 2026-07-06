#pragma once
#include <mutex>
#include <ostream>
#include <string>
#include <string_view>

#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/plugin/i_error_plugin.h"

/**
 * @file log_plugin.h
 * @brief 错误日志插件
 * @details 继承 i_error_plugin_t，将错误上下文格式化后输出到指定 std::ostream。
 *          支持 text / json 两种格式，通过 min_level() 过滤低级别事件。
 *          线程安全：内部用 std::mutex 保护流写入，避免多线程交错输出。
 *          适用场景：开发期调试日志、生产环境错误归档（配合文件流或 syslog 流）。
 *
 *          不内置文件管理逻辑（KISS 原则），调用方负责流的生命周期；
 *          默认输出到 std::cerr，构造时传入自定义流即可重定向。
 * @author yiice
 * @version 3.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */
namespace error_system::plugin {

    /**
     * @brief 错误日志插件
     * @details 将 on_error 收到的 error_context_t 格式化后写入目标流。
     *          - format_t::text：调用 context.to_string()，人类可读
     *          - format_t::json：调用 context.to_json()，机器可读（便于 ELK/Loki 采集）
     * @code
     *   auto plugin = std::make_unique<log_plugin_t>("app_log",
     *                                                 error_level_t::warn,
     *                                                 log_plugin_t::format_t::json,
     *                                                 &file_stream);
     *   plugin_registry_t::instance().register_plugin(std::move(plugin));
     * @endcode
     */
    class log_plugin_t : public i_error_plugin_t {
    public:
        /**
         * @brief 日志输出格式
         */
        enum class format_t {
            text,
            json,
        };

    private:
        std::string name_;
        core::error_level_t min_level_;
        format_t format_;
        std::ostream* stream_;
        mutable std::mutex mutex_;

    public:
        /**
         * @brief 构造日志插件
         * @param name 插件名称（注册时用于去重替换）
         * @param min_level 最低输出级别，低于此级别的事件不输出（默认 warn）
         * @param format 输出格式（默认 text）
         * @param stream 目标输出流指针，nullptr 时使用 std::cerr（调用方需保证流生命周期长于插件）
         */
        explicit log_plugin_t(std::string name = "logger",
                               core::error_level_t min_level = core::error_level_t::warn,
                               format_t format = format_t::text,
                               std::ostream* stream = nullptr) noexcept;

        ~log_plugin_t() noexcept override = default;

        log_plugin_t(const log_plugin_t&) = delete;
        log_plugin_t& operator=(const log_plugin_t&) = delete;
        log_plugin_t(log_plugin_t&&) = delete;
        log_plugin_t& operator=(log_plugin_t&&) = delete;

        /**
         * @brief 错误事件回调（格式化并写入流）
         * @param context 错误上下文
         */
        void on_error(const core::error_context_t& context) noexcept override;

        /**
         * @brief 获取插件名称
         * @return std::string_view 插件名称
         */
        [[nodiscard]] std::string_view name() const noexcept override {
            return name_;
        }

        /**
         * @brief 获取最低输出级别
         * @return core::error_level_t 最低级别
         */
        [[nodiscard]] core::error_level_t min_level() const noexcept override {
            return min_level_;
        }
    };

}  // namespace error_system::plugin
