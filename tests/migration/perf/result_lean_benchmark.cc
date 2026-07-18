/**
 * @file benchmark_result_lean.cc
 * @brief result_t<T, true> (Lean) 与 result_t<T, false> (完整) 性能对比基准
 * @details 对比四个维度：
 *          1. 构造开销：make_success / make_error(code, msg) / make_error(context)
 *          2. 访问开销：value() / error_code() / is_success() / operator bool
 *          3. monadic 传播：map() / and_then() 错误穿透
 *          4. 内存占用：sizeof 编译期对比
 *          全程关闭 validation/stacktrace/location，排除干扰因素。
 */

#include <cstdio>
#include <string>

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

    error_code_t make_bench_code(uint16_t number = 1) {
        return error_code_t{error_level_t::error, system_domain_t::application,
                            subsystem_id_t{1}, module_id_t{1}, error_number_t{number}};
    }

    void disable_optional_features() noexcept {
        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(false);
        feature_flags_t::set_enable_source_location(false);
        feature_flags_t::set_enable_short_filename(false);
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    }

    void prepare_environment() noexcept {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        error_registry_t::instance().set_duplicate_warn_callback(nullptr);
        disable_optional_features();
    }

    error_code_t register_bench_code() noexcept {
        const auto code = make_bench_code();
        error_registry_t::instance().register_error(code, "ERR_BENCH", "基准测试错误码");
        return code;
    }

    template <typename T, bool Lean>
    void make_success_value(benchmark::State& state) {
        for (auto _ : state) {
            auto r = result_t<T, Lean>::make_success(T{42});
            benchmark::DoNotOptimize(r);
        }
        state.SetItemsProcessed(state.iterations());
    }

    template <typename T, bool Lean>
    void make_error_code_msg(benchmark::State& state, const error_code_t& code) {
        for (auto _ : state) {
            auto r = result_t<T, Lean>::make_error(code, "benchmark error");
            benchmark::DoNotOptimize(r);
#ifndef NDEBUG
            (void)r.is_error();
#endif
        }
        state.SetItemsProcessed(state.iterations());
    }

    template <typename T, bool Lean>
    void make_error_from_context(benchmark::State& state, const error_context_t& ctx) {
        for (auto _ : state) {
            auto r = result_t<T, Lean>::make_error(ctx);
            benchmark::DoNotOptimize(r);
#ifndef NDEBUG
            (void)r.is_error();
#endif
        }
        state.SetItemsProcessed(state.iterations());
    }

    template <typename T, bool Lean>
    void access_value_success(benchmark::State& state) {
        auto r = result_t<T, Lean>::make_success(T{42});
        for (auto _ : state) {
            T& v = r.value();
            benchmark::DoNotOptimize(v);
        }
        state.SetItemsProcessed(state.iterations());
    }

    template <typename T, bool Lean>
    void access_error_code_on_error(benchmark::State& state, const error_code_t& code) {
        auto r = result_t<T, Lean>::make_error(code, "bench");
        for (auto _ : state) {
            auto ec = r.error_code();
            benchmark::DoNotOptimize(ec);
        }
        state.SetItemsProcessed(state.iterations());
#ifndef NDEBUG
        (void)r.is_error();
#endif
    }

    template <typename T, bool Lean>
    void check_is_success(benchmark::State& state) {
        auto r = result_t<T, Lean>::make_success(T{42});
        for (auto _ : state) {
            bool ok = r.is_success();
            benchmark::DoNotOptimize(ok);
        }
        state.SetItemsProcessed(state.iterations());
    }

    template <typename T, bool Lean>
    void map_success_propagation(benchmark::State& state) {
        auto r = result_t<T, Lean>::make_success(T{21});
        for (auto _ : state) {
            auto mapped = r.map([](const T& v) { return v * 2; });
            benchmark::DoNotOptimize(mapped);
        }
        state.SetItemsProcessed(state.iterations());
    }

    template <typename T, bool Lean>
    void map_error_propagation(benchmark::State& state, const error_code_t& code) {
        auto r = result_t<T, Lean>::make_error(code, "bench");
        for (auto _ : state) {
            auto mapped = r.map([](const T& v) { return v * 2; });
            benchmark::DoNotOptimize(mapped);
#ifndef NDEBUG
            (void)mapped.is_error();
#endif
        }
        state.SetItemsProcessed(state.iterations());
#ifndef NDEBUG
        (void)r.is_error();
#endif
    }

    template <typename T, bool Lean>
    void and_then_success(benchmark::State& state) {
        auto r = result_t<T, Lean>::make_success(T{10});
        for (auto _ : state) {
            auto next = std::move(r).and_then([](T&& v) {
                return result_t<T, Lean>::make_success(v + 5);
            });
            benchmark::DoNotOptimize(next);
            r = result_t<T, Lean>::make_success(T{10});
        }
        state.SetItemsProcessed(state.iterations());
    }

}  // namespace

class result_lean_fixture_t : public benchmark::Fixture {
public:
    void SetUp(const benchmark::State& /*state*/) override {
        prepare_environment();
        code_ = register_bench_code();
        ctx_ = std::make_unique<error_context_t>(located_code_t{code_}, "基准上下文");
    }

    void TearDown(const benchmark::State& /*state*/) override {
        ctx_.reset();
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
    }

    error_code_t code() const noexcept { return code_; }
    const error_context_t& context() const noexcept { return *ctx_; }

private:
    error_code_t code_{};
    std::unique_ptr<error_context_t> ctx_;
};

BENCHMARK_DEFINE_F(result_lean_fixture_t, make_success_full_int)(benchmark::State& state) {
    make_success_value<int, false>(state);
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, make_success_lean_int)(benchmark::State& state) {
    make_success_value<int, true>(state);
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, make_success_full_string)(benchmark::State& state) {
    make_success_value<std::string, false>(state);
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, make_success_lean_string)(benchmark::State& state) {
    make_success_value<std::string, true>(state);
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, make_error_code_msg_full)(benchmark::State& state) {
    make_error_code_msg<int, false>(state, code());
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, make_error_code_msg_lean)(benchmark::State& state) {
    make_error_code_msg<int, true>(state, code());
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, make_error_from_ctx_full)(benchmark::State& state) {
    make_error_from_context<int, false>(state, context());
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, make_error_from_ctx_lean)(benchmark::State& state) {
    make_error_from_context<int, true>(state, context());
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, access_value_full)(benchmark::State& state) {
    access_value_success<int, false>(state);
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, access_value_lean)(benchmark::State& state) {
    access_value_success<int, true>(state);
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, access_error_code_full)(benchmark::State& state) {
    access_error_code_on_error<int, false>(state, code());
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, access_error_code_lean)(benchmark::State& state) {
    access_error_code_on_error<int, true>(state, code());
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, is_success_full)(benchmark::State& state) {
    check_is_success<int, false>(state);
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, is_success_lean)(benchmark::State& state) {
    check_is_success<int, true>(state);
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, map_success_full)(benchmark::State& state) {
    map_success_propagation<int, false>(state);
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, map_success_lean)(benchmark::State& state) {
    map_success_propagation<int, true>(state);
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, map_error_full)(benchmark::State& state) {
    map_error_propagation<int, false>(state, code());
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, map_error_lean)(benchmark::State& state) {
    map_error_propagation<int, true>(state, code());
}

BENCHMARK_DEFINE_F(result_lean_fixture_t, and_then_success_full)(benchmark::State& state) {
    and_then_success<int, false>(state);
}
BENCHMARK_DEFINE_F(result_lean_fixture_t, and_then_success_lean)(benchmark::State& state) {
    and_then_success<int, true>(state);
}

BENCHMARK_REGISTER_F(result_lean_fixture_t, make_success_full_int)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, make_success_lean_int)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, make_success_full_string)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, make_success_lean_string)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, make_error_code_msg_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, make_error_code_msg_lean)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, make_error_from_ctx_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, make_error_from_ctx_lean)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, access_value_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, access_value_lean)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, access_error_code_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, access_error_code_lean)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, is_success_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, is_success_lean)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, map_success_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, map_success_lean)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, map_error_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, map_error_lean)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, and_then_success_full)->MinTime(0.5);
BENCHMARK_REGISTER_F(result_lean_fixture_t, and_then_success_lean)->MinTime(0.5);

static void memory_footprint_sizes(benchmark::State& state) {
    std::size_t full_int = sizeof(result_t<int, false>);
    std::size_t lean_int = sizeof(result_t<int, true>);
    std::size_t full_str = sizeof(result_t<std::string, false>);
    std::size_t lean_str = sizeof(result_t<std::string, true>);
    std::size_t full_void = sizeof(result_t<void, false>);
    std::size_t lean_void = sizeof(result_t<void, true>);
    for (auto _ : state) {
        benchmark::DoNotOptimize(full_int);
        benchmark::DoNotOptimize(lean_int);
        benchmark::DoNotOptimize(full_str);
        benchmark::DoNotOptimize(lean_str);
        benchmark::DoNotOptimize(full_void);
        benchmark::DoNotOptimize(lean_void);
    }
    state.counters["full_int_bytes"] = static_cast<double>(full_int);
    state.counters["lean_int_bytes"] = static_cast<double>(lean_int);
    state.counters["full_str_bytes"] = static_cast<double>(full_str);
    state.counters["lean_str_bytes"] = static_cast<double>(lean_str);
    state.counters["full_void_bytes"] = static_cast<double>(full_void);
    state.counters["lean_void_bytes"] = static_cast<double>(lean_void);
    state.counters["lean_vs_full_int_ratio"] = static_cast<double>(lean_int) / static_cast<double>(full_int);
    state.counters["lean_vs_full_str_ratio"] = static_cast<double>(lean_str) / static_cast<double>(full_str);
    state.counters["lean_vs_full_void_ratio"] = static_cast<double>(lean_void) / static_cast<double>(full_void);
    state.SetLabel("Lean vs full sizeof comparison (bytes)");
}
BENCHMARK(memory_footprint_sizes)->Iterations(1)->Repetitions(1);

BENCHMARK_MAIN();
