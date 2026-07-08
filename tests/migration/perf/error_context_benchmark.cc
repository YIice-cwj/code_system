/**
 * @file benchmark_error_context.cc
 * @brief error_context_t 全场景性能基准（Google Benchmark 版）
 * @details 覆盖原 scenario_baseline / scenario_stacktrace / scenario_plugin /
 *          scenario_stacktrace_plugin / scenario_full 五种特性开关组合，
 *          通过 Arguments 机制在同一组 BENCHMARK 中横向对比。
 *          新增反序列化基准（from_json / from_binary）。
 *          场景编码：
 *            0 = 基线（栈追踪关 / 位置关 / 无插件）
 *            1 = 栈追踪开 / 位置关 / 无插件
 *            2 = 栈追踪关 / 位置关 / 有插件
 *            3 = 栈追踪开 / 位置关 / 有插件
 *            4 = 全开（栈追踪开 / 位置开 / 有插件）
 */

#include <cstdio>
#include <memory>
#include <string>

#include <benchmark/benchmark.h>

#include "error_system/config/feature_flags.h"
#include "error_system/core/error_context.h"
#include "error_system/core/serializer/error_context_serializer.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/domain/system_domain.h"
#include "error_system/i18n/subsystem_module_catalog.h"
#include "error_system/plugin/i_error_plugin.h"
#include "error_system/plugin/plugin_registry.h"

using error_system::config::feature_flags_t;
using error_system::core::error_context_serializer_t;
using error_system::core::error_context_t;
using error_system::core::error_code_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::located_code_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::i18n::subsystem_module_catalog_t;
using error_system::plugin::i_error_plugin_t;
using error_system::plugin::plugin_registry_t;

namespace {

    error_code_t make_bench_code() noexcept {
        return error_code_t{error_level_t::error, system_domain_t::database,
                            subsystem_id_t{1}, module_id_t{1}, error_number_t{1}};
    }

    class counting_plugin_t : public i_error_plugin_t {
    public:
        std::atomic<long long> count{0};
        std::string_view name() const noexcept override { return "bench_counting"; }
        error_level_t min_level() const noexcept override { return error_level_t::debug; }
        void on_error(const error_context_t& /*ctx*/) noexcept override { ++count; }
    };

    std::unique_ptr<counting_plugin_t> g_plugin;

    void apply_scenario(int scenario) noexcept {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        error_registry_t::instance().set_duplicate_warn_callback(nullptr);
        subsystem_module_catalog_t::instance().clear();
        g_plugin.reset();

        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);

        const bool stacktrace_on = (scenario == 1 || scenario == 3 || scenario == 4);
        const bool location_on = (scenario == 4);
        const bool plugin_on = (scenario == 2 || scenario == 3 || scenario == 4);

        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(stacktrace_on);
        feature_flags_t::set_enable_source_location(location_on);
        feature_flags_t::set_enable_short_filename(location_on);

        const auto code = make_bench_code();
        error_registry_t::instance().register_error(code, "ERR_BENCH_CTX", "基准错误码");
        subsystem_module_catalog_t::instance().register_subsystem_module(
            code.get_subsys(), code.get_module(), "bench_subsys", "bench_module");

        if (plugin_on) {
            g_plugin = std::make_unique<counting_plugin_t>();
            plugin_registry_t::instance().register_plugin_ref(*g_plugin);
        }
    }

    void cleanup_scenario() noexcept {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        subsystem_module_catalog_t::instance().clear();
        g_plugin.reset();
    }

    std::string scenario_label(int scenario) noexcept {
        switch (scenario) {
            case 0: return "baseline(trace=off,loc=off,no_plugin)";
            case 1: return "stacktrace(trace=on,loc=off,no_plugin)";
            case 2: return "plugin(trace=off,loc=off,plugin)";
            case 3: return "trace_plugin(trace=on,loc=off,plugin)";
            case 4: return "full(trace=on,loc=on,plugin)";
            default: return "unknown";
        }
    }

}  // namespace

class context_fixture_t : public benchmark::Fixture {
public:
    error_code_t code_{};
    error_context_t context_{};

    void SetUp(const benchmark::State& state) override {
        const int scenario = static_cast<int>(state.range(0));
        apply_scenario(scenario);
        code_ = make_bench_code();
        context_ = error_context_t{located_code_t{code_}, "基准测试上下文消息"};
    }

    void TearDown(const benchmark::State& /*state*/) override {
        cleanup_scenario();
    }
};

BENCHMARK_DEFINE_F(context_fixture_t, construct)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    for (auto _ : state) {
        error_context_t ctx(code_, "bench construct {}", state.iterations());
        plugin_registry_t::instance().notify_error(ctx);
        benchmark::DoNotOptimize(ctx.get_message().size());
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, construct)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(context_fixture_t, to_string)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    for (auto _ : state) {
        const auto text = context_.to_string();
        benchmark::DoNotOptimize(text.size());
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, to_string)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(context_fixture_t, to_json)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    for (auto _ : state) {
        const auto json = context_.to_json();
        benchmark::DoNotOptimize(json.size());
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, to_json)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(context_fixture_t, to_binary)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    for (auto _ : state) {
        const auto bin = context_.to_binary();
        benchmark::DoNotOptimize(bin.size());
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, to_binary)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(context_fixture_t, copy_construct)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    for (auto _ : state) {
        error_context_t dst(context_.clone());
        benchmark::DoNotOptimize(dst.get_message().size());
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, copy_construct)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(context_fixture_t, move_construct)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    error_context_t src(code_, "移动基准源对象");
    for (auto _ : state) {
        error_context_t dst(std::move(src));
        benchmark::DoNotOptimize(dst.get_message().size());
        src = error_context_t(code_, "移动基准源对象");
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, move_construct)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(context_fixture_t, from_json)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    const std::string json = context_.to_json();
    for (auto _ : state) {
        auto opt = error_context_serializer_t::from_json(json);
        benchmark::DoNotOptimize(opt.has_value());
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, from_json)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(context_fixture_t, from_binary)(benchmark::State& state) {
    const int scenario = static_cast<int>(state.range(0));
    const std::string bin = context_.to_binary();
    for (auto _ : state) {
        auto opt = error_context_serializer_t::from_binary(bin);
        benchmark::DoNotOptimize(opt.has_value());
    }
    state.SetLabel(scenario_label(scenario));
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK_REGISTER_F(context_fixture_t, from_binary)
    ->Arg(0)->Arg(1)->Arg(2)->Arg(3)->Arg(4)
    ->MinTime(0.5);

BENCHMARK_MAIN();
