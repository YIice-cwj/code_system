/**
 * @file benchmark_i18n.cc
 * @brief i18n 翻译模块与子系统/模块目录性能基准（Google Benchmark 版）
 * @details 覆盖原 scenario_i18n 场景：
 *          1. i18n_t 单条/批量注册、查询命中/回退/未命中/按输出 locale
 *          2. subsystem_module_catalog_t 注册/查询命中/回退
 *          3. locale_t 字符串转换
 *          使用 namespace i18n 别名避免与系统 locale_t 冲突。
 */

#include <cstdio>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>

#include "error_system/config/feature_flags.h"
#include "error_system/config/i18n_config.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/core/error_registry.h"
#include "error_system/domain/system_domain.h"
#include "error_system/i18n/i18n.h"
#include "error_system/i18n/locale.h"
#include "error_system/i18n/subsystem_module_catalog.h"
#include "error_system/plugin/plugin_registry.h"

using error_system::config::feature_flags_t;
using error_system::config::i18n_config_t;
using error_system::core::error_code_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::i18n::i18n_t;
using error_system::i18n::subsystem_module_catalog_t;
using error_system::plugin::plugin_registry_t;

namespace i18n = error_system::i18n;

namespace {

    void disable_optional_features() noexcept {
        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(false);
        feature_flags_t::set_enable_source_location(false);
        feature_flags_t::set_enable_short_filename(false);
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    }

    error_code_t prepare_bench_env() noexcept {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        error_registry_t::instance().set_duplicate_warn_callback(nullptr);
        subsystem_module_catalog_t::instance().clear();
        i18n_t::instance().clear_all();
        disable_optional_features();

        i18n_config_t::set_enable_i18n(true);
        i18n_config_t::set_default_locale(i18n::locale_t::zh_CN);
        i18n_config_t::clear_output_locale();

        const error_code_t code{error_level_t::error, system_domain_t::database,
                                subsystem_id_t{1}, module_id_t{1}, error_number_t{1}};
        error_registry_t::instance().register_error(code, "ERR_BENCH_I18N", "i18n 基准错误码");
        i18n_t::instance().register_message(i18n::locale_t::zh_CN, code, "基准测试消息");
        subsystem_module_catalog_t::instance().register_subsystem_module(
            i18n::locale_t::zh_CN, 1, 1, "基准子系统", "基准模块");
        return code;
    }

}  // namespace

class i18n_fixture_t : public benchmark::Fixture {
public:
    error_code_t code{};

    void SetUp(const benchmark::State& /*state*/) override {
        code = prepare_bench_env();
    }

    void TearDown(const benchmark::State& /*state*/) override {
        subsystem_module_catalog_t::instance().clear();
        i18n_t::instance().clear_all();
        error_registry_t::instance().unregister_all();
        plugin_registry_t::instance().clear();
    }
};

BENCHMARK_DEFINE_F(i18n_fixture_t, register_single)(benchmark::State& state) {
    auto& i18n = i18n_t::instance();
    uint16_t i = 2000;
    for (auto _ : state) {
        const error_code_t c{error_level_t::error, system_domain_t::database,
                             subsystem_id_t{1}, module_id_t{1},
                             error_number_t{static_cast<uint16_t>(++i)}};
        i18n.register_message(i18n::locale_t::zh_CN, c, "注册基准消息");
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(i18n_fixture_t, register_single)->MinTime(0.5);

BENCHMARK_DEFINE_F(i18n_fixture_t, get_message_hit)(benchmark::State& state) {
    auto& i18n = i18n_t::instance();
    for (auto _ : state) {
        const auto& msg = i18n.get_message(i18n::locale_t::zh_CN, code);
        benchmark::DoNotOptimize(msg.size());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(i18n_fixture_t, get_message_hit)->MinTime(0.5);

BENCHMARK_DEFINE_F(i18n_fixture_t, get_message_fallback)(benchmark::State& state) {
    auto& i18n = i18n_t::instance();
    for (auto _ : state) {
        const auto& msg = i18n.get_message(i18n::locale_t::en_US, code);
        benchmark::DoNotOptimize(msg.size());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(i18n_fixture_t, get_message_fallback)->MinTime(0.5);

BENCHMARK_DEFINE_F(i18n_fixture_t, get_message_miss)(benchmark::State& state) {
    auto& i18n = i18n_t::instance();
    const error_code_t miss_code{error_level_t::error, system_domain_t::database,
                                 subsystem_id_t{99}, module_id_t{99}, error_number_t{99}};
    for (auto _ : state) {
        const auto& msg = i18n.get_message(i18n::locale_t::zh_CN, miss_code);
        benchmark::DoNotOptimize(msg.size());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(i18n_fixture_t, get_message_miss)->MinTime(0.5);

BENCHMARK_DEFINE_F(i18n_fixture_t, catalog_get_hit)(benchmark::State& state) {
    auto& catalog = subsystem_module_catalog_t::instance();
    for (auto _ : state) {
        auto info = catalog.get_subsystem_module_info(i18n::locale_t::zh_CN, 1, 1);
        benchmark::DoNotOptimize(info.subsystem_name.size());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(i18n_fixture_t, catalog_get_hit)->MinTime(0.5);

BENCHMARK_DEFINE_F(i18n_fixture_t, catalog_get_fallback)(benchmark::State& state) {
    auto& catalog = subsystem_module_catalog_t::instance();
    for (auto _ : state) {
        auto info = catalog.get_subsystem_module_info(i18n::locale_t::ja_JP, 1, 1);
        benchmark::DoNotOptimize(info.subsystem_name.size());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(i18n_fixture_t, catalog_get_fallback)->MinTime(0.5);

BENCHMARK_DEFINE_F(i18n_fixture_t, locale_conversion)(benchmark::State& state) {
    using error_system::i18n::from_string;
    using error_system::i18n::to_string;
    for (auto _ : state) {
        auto v = static_cast<std::size_t>(from_string(to_string(i18n::locale_t::zh_CN)));
        benchmark::DoNotOptimize(v);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(i18n_fixture_t, locale_conversion)->MinTime(0.5);

BENCHMARK_MAIN();
