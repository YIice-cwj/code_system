#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>

#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/plugin/metric_plugin.h"

using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::plugin::metric_plugin_t;
using error_system::plugin::metric_snapshot_t;

namespace {
    error_code_t make_error_code(uint16_t number, error_level_t level = error_level_t::error) {
        return error_code_t{level,
                            system_domain_t::application,
                            subsystem_id_t{1},
                            module_id_t{1},
                            error_number_t{number}};
    }

    error_context_t make_error_context(uint16_t number, error_level_t level = error_level_t::error) {
        return error_context_t::make_minimal(make_error_code(number, level));
    }
}

/**
 * @file metric_plugin_test.cc
 * @brief metric_plugin_t 单元测试
 * @details 验证按码/级别/子系统计数、min_level 过滤、快照导出、重置功能。
 */

TEST(MetricPluginTest, name_returns_constructed_name) {
    metric_plugin_t plugin("custom_metric", error_level_t::error);
    EXPECT_EQ(plugin.name(), "custom_metric");
}

TEST(MetricPluginTest, default_name_is_metric) {
    metric_plugin_t plugin;
    EXPECT_EQ(plugin.name(), "metric");
}

TEST(MetricPluginTest, min_level_returns_constructed_value) {
    metric_plugin_t plugin("m", error_level_t::warn);
    EXPECT_EQ(plugin.min_level(), error_level_t::warn);
}

TEST(MetricPluginTest, on_error_increments_total_count) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(1));
    plugin.on_error(make_error_context(2));
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.total_count, 2u);
}

TEST(MetricPluginTest, on_error_counts_by_level) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(1, error_level_t::debug));
    plugin.on_error(make_error_context(2, error_level_t::error));
    plugin.on_error(make_error_context(3, error_level_t::error));
    plugin.on_error(make_error_context(4, error_level_t::fatal));
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.level_counts[0], 1u);
    EXPECT_EQ(snapshot.level_counts[3], 2u);
    EXPECT_EQ(snapshot.level_counts[4], 1u);
}

TEST(MetricPluginTest, on_error_counts_by_code) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(100));
    plugin.on_error(make_error_context(100));
    plugin.on_error(make_error_context(200));
    metric_snapshot_t snapshot = plugin.snapshot();
    const uint64_t code_100 = make_error_code(100).get_code();
    const uint64_t code_200 = make_error_code(200).get_code();
    EXPECT_EQ(snapshot.code_counts.at(code_100), 2u);
    EXPECT_EQ(snapshot.code_counts.at(code_200), 1u);
}

TEST(MetricPluginTest, on_error_counts_by_subsystem) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(1));
    plugin.on_error(make_error_context(2));
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.subsystem_counts.at(1), 2u);
}

TEST(MetricPluginTest, snapshot_does_not_reset_counts) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(1));
    metric_snapshot_t first = plugin.snapshot();
    metric_snapshot_t second = plugin.snapshot();
    EXPECT_EQ(first.total_count, 1u);
    EXPECT_EQ(second.total_count, 1u);
}

TEST(MetricPluginTest, reset_zeros_all_counts) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(1));
    plugin.on_error(make_error_context(2));
    plugin.reset();
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.total_count, 0u);
    EXPECT_TRUE(snapshot.code_counts.empty());
    EXPECT_TRUE(snapshot.subsystem_counts.empty());
}

TEST(MetricPluginTest, reset_allows_recounting) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(1));
    plugin.reset();
    plugin.on_error(make_error_context(2));
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.total_count, 1u);
}

TEST(MetricPluginTest, min_level_filter_via_registry_not_in_plugin) {
    metric_plugin_t plugin("m", error_level_t::error);
    plugin.on_error(make_error_context(1, error_level_t::debug));
    plugin.on_error(make_error_context(2, error_level_t::error));
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.total_count, 2u);
}

TEST(MetricPluginTest, on_code_increments_total_count) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_code(make_error_code(1));
    plugin.on_code(make_error_code(2));
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.total_count, 2u);
}

TEST(MetricPluginTest, on_code_counts_by_level) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_code(make_error_code(1, error_level_t::error));
    plugin.on_code(make_error_code(2, error_level_t::fatal));
    metric_snapshot_t snapshot = plugin.snapshot();
    EXPECT_EQ(snapshot.level_counts[3], 1u);
    EXPECT_EQ(snapshot.level_counts[4], 1u);
}

TEST(MetricPluginTest, on_code_counts_by_code_and_subsystem) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_code(make_error_code(100));
    plugin.on_code(make_error_code(100));
    plugin.on_code(make_error_code(200));
    metric_snapshot_t snapshot = plugin.snapshot();
    const uint64_t code_100 = make_error_code(100).get_code();
    const uint64_t code_200 = make_error_code(200).get_code();
    EXPECT_EQ(snapshot.code_counts.at(code_100), 2u);
    EXPECT_EQ(snapshot.code_counts.at(code_200), 1u);
    EXPECT_EQ(snapshot.subsystem_counts.at(1), 3u);
}

TEST(MetricPluginTest, on_code_and_on_error_share_same_count_path) {
    metric_plugin_t plugin("m", error_level_t::debug);
    plugin.on_error(make_error_context(1));
    plugin.on_code(make_error_code(1));
    metric_snapshot_t snapshot = plugin.snapshot();
    const uint64_t code_1 = make_error_code(1).get_code();
    EXPECT_EQ(snapshot.total_count, 2u);
    EXPECT_EQ(snapshot.code_counts.at(code_1), 2u);
}
