/**
 * @file plain_error_code_benchmark.cc
 * @brief 传统错误码（plain int）与 error_system 对比基准
 * @details 对比四个维度：
 *          1. 构造开销：plain int 返回 vs result_t<int> vs error_context_t
 *          2. 错误传播：if 检查 vs is_error() 检查
 *          3. 错误输出：printf vs to_string()
 *          4. 内存占用：sizeof 编译期对比
 *          全程关闭 validation/stacktrace/location，模拟传统错误码场景。
 */

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <system_error>

#include <benchmark/benchmark.h>

#include "error_system/config/feature_flags.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/core/result/result.h"
#include "error_system/domain/system_domain.h"
#include "error_system/plugin/plugin_registry.h"

using error_system::config::feature_flags_t;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::located_code_t;
using error_system::core::module_id_t;
using error_system::core::result_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::plugin::plugin_registry_t;

namespace {

    constexpr int PLAIN_ERROR_DB = 5001;

    error_code_t make_bench_code() noexcept {
        return error_code_t{error_level_t::error, system_domain_t::database,
                            subsystem_id_t{1}, module_id_t{1}, error_number_t{1}};
    }

    [[maybe_unused]] void prepare_environment() noexcept {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        error_registry_t::instance().set_duplicate_warn_callback(nullptr);
        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(false);
        feature_flags_t::set_enable_source_location(false);
        feature_flags_t::set_enable_short_filename(false);
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        error_registry_t::instance().register_error(make_bench_code(), "ERR_BENCH", "基准测试错误码");
    }

    // ============ 传统 plain int 错误码 ============

    int plain_db_op(int input) noexcept {
        if (input < 0) { return PLAIN_ERROR_DB; }
        return input * 2;
    }

    int plain_propagate(int input) noexcept {
        int r = plain_db_op(input);
        if (r < 0) { return r; }
        return r + 1;
    }

    // ============ std::error_code ============

    std::error_code make_std_error(int ev) noexcept {
        return std::error_code(ev, std::system_category());
    }

    // ============ error_system result_t ============

    result_t<int> es_db_op(int input) noexcept {
        if (input < 0) {
            return result_t<int>::make_error(make_bench_code(), "db op failed");
        }
        return result_t<int>::make_success(input * 2);
    }

    result_t<int> es_propagate(int input) noexcept {
        auto r = es_db_op(input);
        if (r.is_error()) { return r; }
        return result_t<int>::make_success(r.value() + 1);
    }

    // ============ error_system error_context_t（完整模式） ============

}  // namespace

// ============ 1. 构造开销对比 ============

static void bm_plain_int_success(benchmark::State& state) {
    for (auto _ : state) {
        int r = plain_db_op(42);
        benchmark::DoNotOptimize(r);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_plain_int_success)->MinTime(0.5);

static void bm_plain_int_error(benchmark::State& state) {
    for (auto _ : state) {
        int r = plain_db_op(-1);
        benchmark::DoNotOptimize(r);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_plain_int_error)->MinTime(0.5);

static void bm_std_error_code_success(benchmark::State& state) {
    for (auto _ : state) {
        std::error_code ec = make_std_error(0);
        benchmark::DoNotOptimize(ec);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_std_error_code_success)->MinTime(0.5);

static void bm_std_error_code_error(benchmark::State& state) {
    for (auto _ : state) {
        std::error_code ec = make_std_error(PLAIN_ERROR_DB);
        benchmark::DoNotOptimize(ec);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_std_error_code_error)->MinTime(0.5);

static void bm_result_success(benchmark::State& state) {
    for (auto _ : state) {
        auto r = result_t<int>::make_success(42);
        benchmark::DoNotOptimize(r);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_result_success)->MinTime(0.5);

static void bm_result_error_code_msg(benchmark::State& state) {
    const auto code = make_bench_code();
    for (auto _ : state) {
        auto r = result_t<int>::make_error(code, "db op failed");
        benchmark::DoNotOptimize(r.is_error());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_result_error_code_msg)->MinTime(0.5);

static void bm_result_error_context(benchmark::State& state) {
    const auto code = make_bench_code();
    for (auto _ : state) {
        error_context_t ctx{located_code_t{code}, "db op failed"};
        auto r = result_t<int>::make_error(std::move(ctx));
        benchmark::DoNotOptimize(r.is_error());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_result_error_context)->MinTime(0.5);

static void bm_error_context_direct(benchmark::State& state) {
    const auto code = make_bench_code();
    for (auto _ : state) {
        error_context_t ctx{located_code_t{code}, "db op failed"};
        benchmark::DoNotOptimize(ctx.is_error());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_error_context_direct)->MinTime(0.5);

// ============ 2. 错误传播对比（含一次调用 + 检查） ============

static void bm_plain_propagate_success(benchmark::State& state) {
    for (auto _ : state) {
        int r = plain_propagate(42);
        benchmark::DoNotOptimize(r);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_plain_propagate_success)->MinTime(0.5);

static void bm_plain_propagate_error(benchmark::State& state) {
    for (auto _ : state) {
        int r = plain_propagate(-1);
        benchmark::DoNotOptimize(r);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_plain_propagate_error)->MinTime(0.5);

static void bm_result_propagate_success(benchmark::State& state) {
    for (auto _ : state) {
        auto r = es_propagate(42);
        benchmark::DoNotOptimize(r);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_result_propagate_success)->MinTime(0.5);

static void bm_result_propagate_error(benchmark::State& state) {
    for (auto _ : state) {
        auto r = es_propagate(-1);
        benchmark::DoNotOptimize(r);
#ifndef NDEBUG
        (void)r.is_error();
#endif
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_result_propagate_error)->MinTime(0.5);

// ============ 3. 错误输出对比 ============

static void bm_plain_printf(benchmark::State& state) {
    char buf[128];
    for (auto _ : state) {
        std::snprintf(buf, sizeof(buf), "error: db op failed (code=%d)", PLAIN_ERROR_DB);
        benchmark::DoNotOptimize(buf);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_plain_printf)->MinTime(0.5);

static void bm_result_to_string(benchmark::State& state) {
    const auto code = make_bench_code();
    error_context_t ctx{located_code_t{code}, "db op failed"};
    for (auto _ : state) {
        const auto s = ctx.to_string();
        benchmark::DoNotOptimize(s.size());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_result_to_string)->MinTime(0.5);

static void bm_result_to_json(benchmark::State& state) {
    const auto code = make_bench_code();
    error_context_t ctx{located_code_t{code}, "db op failed"};
    for (auto _ : state) {
        const auto s = ctx.to_json();
        benchmark::DoNotOptimize(s.size());
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(bm_result_to_json)->MinTime(0.5);

// ============ 4. sizeof 编译期对比 ============

static void bm_sizeof_display(benchmark::State& state) {
    constexpr size_t sz_int = sizeof(int);
    constexpr size_t sz_error_code = sizeof(std::error_code);
    constexpr size_t sz_error_context = sizeof(error_context_t);
    constexpr size_t sz_result_int = sizeof(result_t<int>);
    constexpr size_t sz_result_int_lean = sizeof(result_t<int, true>);

    state.counters["sizeof_int"] = sz_int;
    state.counters["sizeof_std_error_code"] = sz_error_code;
    state.counters["sizeof_error_context"] = sz_error_context;
    state.counters["sizeof_result_int_full"] = sz_result_int;
    state.counters["sizeof_result_int_lean"] = sz_result_int_lean;

    for (auto _ : state) {
        size_t v = sz_int;
        benchmark::DoNotOptimize(v);
    }
}
BENCHMARK(bm_sizeof_display)->Iterations(1);

namespace {
    /// 全局初始化器：在 main 前关闭所有可选特性，确保公平对比
    struct environment_setup_t {
        environment_setup_t() noexcept { prepare_environment(); }
    } environment_setup_instance;
}

BENCHMARK_MAIN();
