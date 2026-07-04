#include "error_system/plugin/metric_plugin.h"

/**
 * @file metric_plugin.cc
 * @brief 错误指标统计插件实现
 * @details 按 错误码/级别/子系统 三维度计数，mutex 保护，noexcept 安全。
 * @author yiice
 * @version 1.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */

#include <cstdio>
#include <new>
#include <utility>

namespace error_system::plugin {

    metric_plugin_t::metric_plugin_t(std::string name, core::error_level_t min_level) noexcept
        : name_(std::move(name)), min_level_(min_level) {}

    std::string_view metric_plugin_t::name() const noexcept {
        return name_;
    }

    core::error_level_t metric_plugin_t::min_level() const noexcept {
        return min_level_;
    }

    void metric_plugin_t::on_error(const core::error_context_t& context) noexcept {
        const uint64_t code = context.get_code().get_code();
        const uint8_t level_index = core::to_int(context.get_code().get_level());
        const uint16_t subsystem = context.get_code().get_subsys();

        try {
            std::lock_guard<std::mutex> lock(mutex_);
            ++total_count_;
            if (level_index < level_counts_.size()) {
                ++level_counts_[level_index];
            }
            ++code_counts_[code];
            ++subsystem_counts_[subsystem];
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[metric_plugin] on_error: std::bad_alloc\n");
        }
    }

    metric_snapshot_t metric_plugin_t::snapshot() const noexcept {
        metric_snapshot_t snapshot;
        try {
            std::lock_guard<std::mutex> lock(mutex_);
            snapshot.total_count = total_count_;
            snapshot.level_counts = level_counts_;
            snapshot.code_counts = code_counts_;
            snapshot.subsystem_counts = subsystem_counts_;
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[metric_plugin] snapshot: std::bad_alloc\n");
        }
        return snapshot;
    }

    void metric_plugin_t::reset() noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        total_count_ = 0;
        level_counts_.fill(0);
        code_counts_.clear();
        subsystem_counts_.clear();
    }

}  // namespace error_system::plugin
