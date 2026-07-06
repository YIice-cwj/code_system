#pragma once
#include <array>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/plugin/i_error_plugin.h"

/**
 * @file metric_plugin.h
 * @brief 错误指标统计插件
 * @details 继承 i_error_plugin_t，按错误码/级别/子系统维度统计错误次数，
 *          提供 snapshot() 导出快照与 reset() 重置统计。
 *          线程安全：内部用 std::mutex 保护，on_error()/snapshot()/reset() 可并发调用。
 *          适用场景：错误率监控、热点错误码定位、子系统健康度评估。
 * @author yiice
 * @version 3.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */
namespace error_system::plugin {

    /**
     * @brief 错误指标快照
     * @details metric_plugin_t::snapshot() 返回的不可变统计快照，拷贝后可独立访问。
     *          level_counts 索引与 error_level_t 数值对应：0=debug, 1=info, 2=warn, 3=error, 4=fatal。
     */
    struct metric_snapshot_t {
        uint64_t total_count{0};
        std::array<uint64_t, 5> level_counts{0, 0, 0, 0, 0};
        std::unordered_map<uint64_t, uint64_t> code_counts{};
        std::unordered_map<uint16_t, uint64_t> subsystem_counts{};
    };

    /**
     * @brief 错误指标统计插件
     * @details 按 error_code_t::get_code()（完整 64 位码）、get_level()、get_subsys() 三个维度计数。
     *          通过 min_level() 过滤低级别事件，避免 debug 噪音污染统计。
     *          使用 std::mutex 保护内部状态，on_error 路径为 O(1) 原子递增 + 一次 map 查找。
     * @code
     *   auto plugin = std::make_unique<metric_plugin_t>("app_metric", error_level_t::error);
     *   plugin_registry_t::instance().register_plugin(std::move(plugin));
     *   // ... 运行业务 ...
     *   auto snapshot = static_cast<metric_plugin_t*>(plugin_ptr)->snapshot();
     *   std::printf("total=%llu\n", static_cast<unsigned long long>(snapshot.total_count));
     * @endcode
     */
    class metric_plugin_t : public i_error_plugin_t {
    private:
        std::string name_;
        core::error_level_t min_level_;
        mutable std::mutex mutex_;
        uint64_t total_count_{0};
        std::array<uint64_t, 5> level_counts_{0, 0, 0, 0, 0};
        std::unordered_map<uint64_t, uint64_t> code_counts_;
        std::unordered_map<uint16_t, uint64_t> subsystem_counts_;

    public:
        /**
         * @brief 构造指标插件
         * @param name 插件名称（注册时用于去重替换）
         * @param min_level 最低关注级别，低于此级别的错误事件不计数（默认 error）
         */
        explicit metric_plugin_t(std::string name = "metric",
                                  core::error_level_t min_level = core::error_level_t::error) noexcept
            : name_(std::move(name)), min_level_(min_level) {}

        ~metric_plugin_t() noexcept override = default;

        metric_plugin_t(const metric_plugin_t&) = delete;
        metric_plugin_t& operator=(const metric_plugin_t&) = delete;
        metric_plugin_t(metric_plugin_t&&) = delete;
        metric_plugin_t& operator=(metric_plugin_t&&) = delete;

        /**
         * @brief 错误事件回调（更新计数）
         * @param context 错误上下文
         */
        void on_error(const core::error_context_t& context) noexcept override;

        /**
         * @brief 导出当前统计快照
         * @return metric_snapshot_t 统计快照（拷贝）
         */
        [[nodiscard]] metric_snapshot_t snapshot() const noexcept;

        /**
         * @brief 重置所有计数为零
         */
        void reset() noexcept;

        /**
         * @brief 获取插件名称
         * @return std::string_view 插件名称
         */
        [[nodiscard]] std::string_view name() const noexcept override {
            return name_;
        }

        /**
         * @brief 获取最低关注级别
         * @return core::error_level_t 最低级别
         */
        [[nodiscard]] core::error_level_t min_level() const noexcept override {
            return min_level_;
        }
    };

}  // namespace error_system::plugin
