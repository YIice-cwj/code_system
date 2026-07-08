#include "error_system/plugin/metric_plugin.h"

/**
 * @file metric_plugin.cc
 * @brief 错误指标统计插件实现
 * @details 按 错误码/级别/子系统 三维度计数，mutex 保护，noexcept 安全。
 *          on_error 与 on_code 共享 count_code_ 实现，保证统计路径一致。
 * @author yiice
 * @version 4.4.0
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <cstdio>
#include <new>
#include <utility>

#include "error_system/utils/bad_alloc_handler.h"

namespace error_system::plugin {

    void metric_plugin_t::count_code_(core::error_code_t code) noexcept {
        const uint64_t raw_code = code.get_code();
        const uint8_t level_index = core::to_int(code.get_level());
        const uint16_t subsystem = code.get_subsys();

        try {
            std::lock_guard<std::mutex> lock(mutex_);
            ++total_count_;
            if (level_index < level_counts_.size()) {
                ++level_counts_[level_index];
            }
            ++code_counts_[raw_code];
            ++subsystem_counts_[subsystem];
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("metric_plugin", "count_code_");
        }
    }

    void metric_plugin_t::on_error(const core::error_context_t& context) noexcept {
        count_code_(context.get_code());
    }

    void metric_plugin_t::on_code(core::error_code_t code) noexcept {
        count_code_(code);
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
            utils::report_bad_alloc("metric_plugin", "snapshot");
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
