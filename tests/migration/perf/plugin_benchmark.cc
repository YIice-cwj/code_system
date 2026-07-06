/**
 * @file plugin_benchmark.cc
 * @brief 插件层性能基准：路由分发、去重采样、通知模式对比、真实插件开销、多插件叠加曲线
 * @details 覆盖以下维度：
 *          1. error_router_plugin_t：按错误码/模块组/域/未命中 四级分发
 *          2. error_dedup_sampler_t：全部放行/纯去重/纯采样/去重+采样
 *          3. 通知模式对比（null 插件）：sync vs async_queue vs sync_deferred
 *          4. 单真实插件开销：log_plugin_t / metric_plugin_t / error_router_plugin_t
 *          5. 多插件叠加曲线：null×{1,2,4,8} 测 registry 派发底噪 + 真实组合（log+metric / +router / +dedup）
 *          6. 通知模式 × 真实插件矩阵：3 模式 × 3 插件 = 9 组合
 *          7. 注册/注销开销：1/4/16/64 个插件的 register_plugin / unregister_plugin
 *          全程关闭栈追踪/源位置/校验，排除干扰因素。
 */

#include <atomic>
#include <cstdio>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <benchmark/benchmark.h>

#include "error_system/config/feature_flags.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/domain/system_domain.h"
#include "error_system/plugin/error_dedup_sampler.h"
#include "error_system/plugin/error_router_plugin.h"
#include "error_system/plugin/i_error_plugin.h"
#include "error_system/plugin/log_plugin.h"
#include "error_system/plugin/metric_plugin.h"
#include "error_system/plugin/plugin_registry.h"

using error_system::config::feature_flags_t;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::located_code_t;
using error_system::core::module_group_id_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::plugin::error_dedup_sampler_t;
using error_system::plugin::error_router_plugin_t;
using error_system::plugin::i_error_plugin_t;
using error_system::plugin::log_plugin_t;
using error_system::plugin::metric_plugin_t;
using error_system::plugin::plugin_registry_t;

namespace {

    /**
     * @brief 空输出流缓冲，用于 log_plugin_t 的输出重定向（零开销丢弃）
     * @details 避免 log_plugin_t 输出到 std::cerr 干扰 benchmark，且不产生实际 IO 开销
     */
    class null_streambuf_t : public std::streambuf {
    protected:
        int_type overflow(int_type c) override { return c; }
        std::streamsize xsputn(const char_type* /*s*/, std::streamsize n) override { return n; }
    };

    /**
     * @brief 空操作插件，用于隔离测 plugin_registry 派发循环本身的开销
     * @details on_error 无任何操作，min_level 返回 debug（接收所有级别）
     */
    class null_plugin_t : public i_error_plugin_t {
    public:
        explicit null_plugin_t(std::string name) noexcept : name_(std::move(name)) {}
        std::string_view name() const noexcept override { return name_; }
        error_level_t min_level() const noexcept override { return error_level_t::debug; }
        void on_error(const error_context_t& /*ctx*/) noexcept override {}
    private:
        std::string name_;
    };

    /**
     * @brief 包装 error_dedup_sampler_t 为 i_error_plugin_t，用于多插件组合场景
     * @details on_error 内部调用 should_be_forwarded，通过则无操作（模拟"放行后由后续插件处理"）
     */
    class dedup_plugin_wrapper_t : public i_error_plugin_t {
    public:
        dedup_plugin_wrapper_t() noexcept = default;
        std::string_view name() const noexcept override { return "dedup_wrapper"; }
        error_level_t min_level() const noexcept override { return error_level_t::debug; }
        void on_error(const error_context_t& ctx) noexcept override {
            sampler_.set_dedup_window_ms(1000);
            sampler_.set_sample_rate(1.0);
            [[maybe_unused]] const bool forwarded = sampler_.should_be_forwarded(ctx);
        }
        error_dedup_sampler_t& sampler() noexcept { return sampler_; }
    private:
        error_dedup_sampler_t sampler_;
    };

    void disable_optional_features() noexcept {
        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(false);
        feature_flags_t::set_enable_source_location(false);
        feature_flags_t::set_enable_short_filename(false);
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    }

    error_code_t make_code(uint16_t subsys = 1, uint16_t module = 1, uint16_t number = 1) noexcept {
        return error_code_t{error_level_t::error, system_domain_t::database,
                            subsystem_id_t{subsys}, module_id_t{module}, error_number_t{number}};
    }

    void prepare_registry() noexcept {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        error_registry_t::instance().set_duplicate_warn_callback(nullptr);
        disable_optional_features();
    }

}  // namespace

class router_fixture_t : public benchmark::Fixture {
public:
    error_code_t code_hit{};
    error_code_t code_module_only{};
    error_code_t code_domain_only{};
    error_code_t code_miss{};
    error_context_t ctx_hit{};
    error_context_t ctx_module{};
    error_context_t ctx_domain{};
    error_context_t ctx_miss{};

    void SetUp(const benchmark::State& /*state*/) override {
        prepare_registry();
        auto& router = error_router_plugin_t::instance();

        code_hit = make_code(1, 1, 1);
        code_module_only = make_code(1, 1, 2);
        code_domain_only = make_code(2, 2, 1);
        code_miss = make_code(3, 3, 1);

        ctx_hit = error_context_t{located_code_t{code_hit}, "hit"};
        ctx_module = error_context_t{located_code_t{code_module_only}, "module"};
        ctx_domain = error_context_t{located_code_t{code_domain_only}, "domain"};
        ctx_miss = error_context_t{located_code_t{code_miss}, "miss"};

        auto handler = [](const error_context_t&) noexcept {};
        router.register_handler_by_code(code_hit, handler);
        router.register_handler_by_module_group_id(code_module_only.get_module_group_id(), handler);
        router.register_handler_by_domain(system_domain_t::database, handler);
    }

    void TearDown(const benchmark::State& /*state*/) override {
        auto& router = error_router_plugin_t::instance();
        router.unregister_handler_by_code(code_hit);
        router.unregister_handler_by_module_group_id(code_module_only.get_module_group_id());
        router.unregister_handler_by_domain(system_domain_t::database);
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
    }
};

BENCHMARK_DEFINE_F(router_fixture_t, dispatch_by_code)(benchmark::State& state) {
    auto& router = error_router_plugin_t::instance();
    for (auto _ : state) {
        router.on_error(ctx_hit);
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("by_code (O(1) hash)");
}
BENCHMARK_REGISTER_F(router_fixture_t, dispatch_by_code)->MinTime(0.5);

BENCHMARK_DEFINE_F(router_fixture_t, dispatch_by_module_group)(benchmark::State& state) {
    auto& router = error_router_plugin_t::instance();
    for (auto _ : state) {
        router.on_error(ctx_module);
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("by_module_group (fallback)");
}
BENCHMARK_REGISTER_F(router_fixture_t, dispatch_by_module_group)->MinTime(0.5);

BENCHMARK_DEFINE_F(router_fixture_t, dispatch_by_domain)(benchmark::State& state) {
    auto& router = error_router_plugin_t::instance();
    for (auto _ : state) {
        router.on_error(ctx_domain);
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("by_domain (fallback)");
}
BENCHMARK_REGISTER_F(router_fixture_t, dispatch_by_domain)->MinTime(0.5);

BENCHMARK_DEFINE_F(router_fixture_t, dispatch_miss)(benchmark::State& state) {
    auto& router = error_router_plugin_t::instance();
    for (auto _ : state) {
        router.on_error(ctx_miss);
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("miss (full 3-level lookup)");
}
BENCHMARK_REGISTER_F(router_fixture_t, dispatch_miss)->MinTime(0.5);

BENCHMARK_DEFINE_F(router_fixture_t, register_handler)(benchmark::State& state) {
    auto& router = error_router_plugin_t::instance();
    auto handler = [](const error_context_t&) noexcept {};
    uint16_t i = 5000;
    for (auto _ : state) {
        const error_code_t c = make_code(1, 1, static_cast<uint16_t>(++i));
        router.register_handler_by_code(c, handler);
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("register_handler_by_code");
}
BENCHMARK_REGISTER_F(router_fixture_t, register_handler)->MinTime(0.5);

class dedup_fixture_t : public benchmark::Fixture {
public:
    error_dedup_sampler_t sampler{};
    error_code_t code{};
    error_context_t ctx{};

    void SetUp(const benchmark::State& /*state*/) override {
        prepare_registry();
        code = make_code();
        ctx = error_context_t{located_code_t{code}, "dedup bench"};
        sampler.set_dedup_window_ms(0);
        sampler.set_sample_rate(1.0);
        sampler.clear_dedup_cache();
        sampler.reset_stats();
    }

    void TearDown(const benchmark::State& /*state*/) override {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
    }
};

BENCHMARK_DEFINE_F(dedup_fixture_t, all_forward)(benchmark::State& state) {
    sampler.set_dedup_window_ms(0);
    sampler.set_sample_rate(1.0);
    sampler.clear_dedup_cache();
    for (auto _ : state) {
        benchmark::DoNotOptimize(sampler.should_be_forwarded(ctx));
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("no dedup, rate=1.0 (all pass)");
}
BENCHMARK_REGISTER_F(dedup_fixture_t, all_forward)->MinTime(0.5);

BENCHMARK_DEFINE_F(dedup_fixture_t, dedup_window_only)(benchmark::State& state) {
    sampler.set_dedup_window_ms(86400000);
    sampler.set_sample_rate(1.0);
    sampler.clear_dedup_cache();
    for (auto _ : state) {
        benchmark::DoNotOptimize(sampler.should_be_forwarded(ctx));
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("dedup=1day, rate=1.0");
}
BENCHMARK_REGISTER_F(dedup_fixture_t, dedup_window_only)->MinTime(0.5);

BENCHMARK_DEFINE_F(dedup_fixture_t, sample_only)(benchmark::State& state) {
    sampler.set_dedup_window_ms(0);
    sampler.set_sample_rate(0.1);
    for (auto _ : state) {
        benchmark::DoNotOptimize(sampler.should_be_forwarded(ctx));
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("no dedup, rate=0.1 (10% pass)");
}
BENCHMARK_REGISTER_F(dedup_fixture_t, sample_only)->MinTime(0.5);

BENCHMARK_DEFINE_F(dedup_fixture_t, dedup_combined)(benchmark::State& state) {
    sampler.set_dedup_window_ms(86400000);
    sampler.set_sample_rate(0.1);
    sampler.clear_dedup_cache();
    for (auto _ : state) {
        benchmark::DoNotOptimize(sampler.should_be_forwarded(ctx));
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel("dedup=1day + rate=0.1");
}
BENCHMARK_REGISTER_F(dedup_fixture_t, dedup_combined)->MinTime(0.5);

class notify_fixture_t : public benchmark::Fixture {
public:
    static constexpr int NOTIFY_MODE_SYNC = 0;
    static constexpr int NOTIFY_MODE_ASYNC = 1;
    static constexpr int NOTIFY_MODE_DEFERRED = 2;

    error_code_t code{};
    std::unique_ptr<i_error_plugin_t> plugin_holder{};

    void SetUp(const benchmark::State& state) override {
        prepare_registry();
        code = make_code();
        const int mode = static_cast<int>(state.range(0));

        plugin_holder = std::make_unique<null_plugin_t>("bench_null");
        plugin_registry_t::instance().register_plugin_ref(*plugin_holder);

        if (mode == NOTIFY_MODE_SYNC) {
            feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        } else if (mode == NOTIFY_MODE_ASYNC) {
            feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::async_queue);
        } else {
            feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
        }
    }

    void TearDown(const benchmark::State& /*state*/) override {
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        plugin_holder.reset();
    }
};

BENCHMARK_DEFINE_F(notify_fixture_t, notify)(benchmark::State& state) {
    const int mode = static_cast<int>(state.range(0));
    for (auto _ : state) {
        error_context_t ctx{located_code_t{code}, "notify bench"};
        (void)ctx;
    }
    if (mode == NOTIFY_MODE_ASYNC) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (mode == NOTIFY_MODE_DEFERRED) {
        plugin_registry_t::instance().flush_deferred_notifications();
    }
    state.SetItemsProcessed(state.iterations());
    state.SetLabel(mode == 0 ? "sync" : (mode == 1 ? "async_queue" : "sync_deferred"));
}
BENCHMARK_REGISTER_F(notify_fixture_t, notify)
    ->Arg(0)->Arg(1)->Arg(2)
    ->MinTime(0.5);

// ===== 4. 单真实插件开销：log / metric / router =====

class single_plugin_fixture_t : public benchmark::Fixture {
public:
    static constexpr int PLUGIN_LOG = 0;
    static constexpr int PLUGIN_METRIC = 1;
    static constexpr int PLUGIN_ROUTER = 2;

    error_code_t code{};
    error_context_t ctx{};
    null_streambuf_t null_buf{};
    std::ostream null_stream{&null_buf};
    std::unique_ptr<log_plugin_t> log_plugin;
    std::unique_ptr<metric_plugin_t> metric_plugin;

    void SetUp(const benchmark::State& state) override {
        prepare_registry();
        code = make_code(1, 1, 1);
        ctx = error_context_t{located_code_t{code}, "single plugin bench"};
        const int plugin_type = static_cast<int>(state.range(0));

        if (plugin_type == PLUGIN_LOG) {
            log_plugin = std::make_unique<log_plugin_t>(
                "bench_log", error_level_t::debug, log_plugin_t::format_t::text, &null_stream);
            plugin_registry_t::instance().register_plugin_ref(*log_plugin);
        } else if (plugin_type == PLUGIN_METRIC) {
            metric_plugin = std::make_unique<metric_plugin_t>("bench_metric", error_level_t::debug);
            plugin_registry_t::instance().register_plugin_ref(*metric_plugin);
        } else {
            auto& router = error_router_plugin_t::instance();
            router.register_handler_by_code(code, [](const error_context_t&) noexcept {});
            plugin_registry_t::instance().register_plugin_ref(router);
        }
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    }

    void TearDown(const benchmark::State& state) override {
        const int plugin_type = static_cast<int>(state.range(0));
        if (plugin_type == PLUGIN_ROUTER) {
            error_router_plugin_t::instance().unregister_handler_by_code(code);
        }
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        log_plugin.reset();
        metric_plugin.reset();
    }
};

BENCHMARK_DEFINE_F(single_plugin_fixture_t, on_error)(benchmark::State& state) {
    const int plugin_type = static_cast<int>(state.range(0));
    auto& registry = plugin_registry_t::instance();
    for (auto _ : state) {
        registry.notify_error(ctx);
    }
    state.SetItemsProcessed(state.iterations());
    const char* labels[] = {"log_plugin", "metric_plugin", "router_plugin"};
    state.SetLabel(labels[plugin_type]);
}
BENCHMARK_REGISTER_F(single_plugin_fixture_t, on_error)
    ->Arg(0)->Arg(1)->Arg(2)
    ->MinTime(0.5);

// ===== 5. 多插件叠加曲线 =====

class null_scaling_fixture_t : public benchmark::Fixture {
public:
    error_code_t code{};
    error_context_t ctx{};
    std::vector<std::unique_ptr<null_plugin_t>> plugins;

    void SetUp(const benchmark::State& state) override {
        prepare_registry();
        code = make_code();
        ctx = error_context_t{located_code_t{code}, "null scaling bench"};
        const size_t count = static_cast<size_t>(state.range(0));
        plugins.clear();
        plugins.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            plugins.push_back(std::make_unique<null_plugin_t>("null_" + std::to_string(i)));
            plugin_registry_t::instance().register_plugin_ref(*plugins.back());
        }
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    }

    void TearDown(const benchmark::State& /*state*/) override {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        plugins.clear();
    }
};

BENCHMARK_DEFINE_F(null_scaling_fixture_t, notify_all)(benchmark::State& state) {
    auto& registry = plugin_registry_t::instance();
    for (auto _ : state) {
        registry.notify_error(ctx);
    }
    state.SetItemsProcessed(state.iterations());
    const int count = static_cast<int>(state.range(0));
    state.SetLabel(std::to_string(count) + " null plugins");
}
BENCHMARK_REGISTER_F(null_scaling_fixture_t, notify_all)
    ->Arg(1)->Arg(2)->Arg(4)->Arg(8)
    ->MinTime(0.5);

class real_combo_fixture_t : public benchmark::Fixture {
public:
    static constexpr int COMBO_LOG_METRIC = 0;
    static constexpr int COMBO_LOG_METRIC_ROUTER = 1;
    static constexpr int COMBO_LOG_METRIC_ROUTER_DEDUP = 2;

    error_code_t code{};
    error_context_t ctx{};
    null_streambuf_t null_buf{};
    std::ostream null_stream{&null_buf};
    std::unique_ptr<log_plugin_t> log_plugin;
    std::unique_ptr<metric_plugin_t> metric_plugin;
    std::unique_ptr<dedup_plugin_wrapper_t> dedup_plugin;

    void SetUp(const benchmark::State& state) override {
        prepare_registry();
        code = make_code(1, 1, 1);
        ctx = error_context_t{located_code_t{code}, "real combo bench"};
        const int combo = static_cast<int>(state.range(0));

        log_plugin = std::make_unique<log_plugin_t>(
            "combo_log", error_level_t::debug, log_plugin_t::format_t::text, &null_stream);
        metric_plugin = std::make_unique<metric_plugin_t>("combo_metric", error_level_t::debug);
        plugin_registry_t::instance().register_plugin_ref(*log_plugin);
        plugin_registry_t::instance().register_plugin_ref(*metric_plugin);

        if (combo >= COMBO_LOG_METRIC_ROUTER) {
            auto& router = error_router_plugin_t::instance();
            router.register_handler_by_code(code, [](const error_context_t&) noexcept {});
            plugin_registry_t::instance().register_plugin_ref(router);
        }
        if (combo >= COMBO_LOG_METRIC_ROUTER_DEDUP) {
            dedup_plugin = std::make_unique<dedup_plugin_wrapper_t>();
            plugin_registry_t::instance().register_plugin_ref(*dedup_plugin);
        }
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    }

    void TearDown(const benchmark::State& state) override {
        const int combo = static_cast<int>(state.range(0));
        if (combo >= COMBO_LOG_METRIC_ROUTER) {
            error_router_plugin_t::instance().unregister_handler_by_code(code);
        }
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        log_plugin.reset();
        metric_plugin.reset();
        dedup_plugin.reset();
    }
};

BENCHMARK_DEFINE_F(real_combo_fixture_t, notify_all)(benchmark::State& state) {
    auto& registry = plugin_registry_t::instance();
    for (auto _ : state) {
        registry.notify_error(ctx);
    }
    state.SetItemsProcessed(state.iterations());
    const int combo = static_cast<int>(state.range(0));
    const char* labels[] = {"log+metric", "log+metric+router", "log+metric+router+dedup"};
    state.SetLabel(labels[combo]);
}
BENCHMARK_REGISTER_F(real_combo_fixture_t, notify_all)
    ->Arg(0)->Arg(1)->Arg(2)
    ->MinTime(0.5);

// ===== 6. 通知模式 × 真实插件矩阵 =====

class notify_mode_plugin_fixture_t : public benchmark::Fixture {
public:
    static constexpr int MODE_SYNC = 0;
    static constexpr int MODE_ASYNC = 1;
    static constexpr int MODE_DEFERRED = 2;
    static constexpr int PLUGIN_LOG = 0;
    static constexpr int PLUGIN_METRIC = 1;
    static constexpr int PLUGIN_ROUTER = 2;

    error_code_t code{};
    null_streambuf_t null_buf{};
    std::ostream null_stream{&null_buf};
    std::unique_ptr<log_plugin_t> log_plugin;
    std::unique_ptr<metric_plugin_t> metric_plugin;

    void SetUp(const benchmark::State& state) override {
        prepare_registry();
        code = make_code(1, 1, 1);
        const int mode = static_cast<int>(state.range(0));
        const int plugin_type = static_cast<int>(state.range(1));

        if (plugin_type == PLUGIN_LOG) {
            log_plugin = std::make_unique<log_plugin_t>(
                "mode_log", error_level_t::debug, log_plugin_t::format_t::text, &null_stream);
            plugin_registry_t::instance().register_plugin_ref(*log_plugin);
        } else if (plugin_type == PLUGIN_METRIC) {
            metric_plugin = std::make_unique<metric_plugin_t>("mode_metric", error_level_t::debug);
            plugin_registry_t::instance().register_plugin_ref(*metric_plugin);
        } else {
            auto& router = error_router_plugin_t::instance();
            router.register_handler_by_code(code, [](const error_context_t&) noexcept {});
            plugin_registry_t::instance().register_plugin_ref(router);
        }

        if (mode == MODE_SYNC) {
            feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        } else if (mode == MODE_ASYNC) {
            feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::async_queue);
        } else {
            feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
        }
    }

    void TearDown(const benchmark::State& state) override {
        const int mode = static_cast<int>(state.range(0));
        const int plugin_type = static_cast<int>(state.range(1));
        if (plugin_type == PLUGIN_ROUTER) {
            error_router_plugin_t::instance().unregister_handler_by_code(code);
        }
        if (mode == MODE_DEFERRED) {
            plugin_registry_t::instance().flush_deferred_notifications();
        }
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        log_plugin.reset();
        metric_plugin.reset();
    }
};

BENCHMARK_DEFINE_F(notify_mode_plugin_fixture_t, notify)(benchmark::State& state) {
    const int mode = static_cast<int>(state.range(0));
    for (auto _ : state) {
        error_context_t ctx{located_code_t{code}, "mode x plugin bench"};
        (void)ctx;
    }
    if (mode == MODE_ASYNC) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    if (mode == MODE_DEFERRED) {
        plugin_registry_t::instance().flush_deferred_notifications();
    }
    state.SetItemsProcessed(state.iterations());
    const char* mode_names[] = {"sync", "async", "deferred"};
    const char* plugin_names[] = {"log", "metric", "router"};
    state.SetLabel(std::string(mode_names[mode]) + "+" + plugin_names[static_cast<int>(state.range(1))]);
}
BENCHMARK_REGISTER_F(notify_mode_plugin_fixture_t, notify)
    ->Args({0, 0})->Args({0, 1})->Args({0, 2})
    ->Args({1, 0})->Args({1, 1})->Args({1, 2})
    ->Args({2, 0})->Args({2, 1})->Args({2, 2})
    ->MinTime(0.5);

// ===== 7. 注册/注销开销 =====

class register_scaling_fixture_t : public benchmark::Fixture {
public:
    void SetUp(const benchmark::State& /*state*/) override {
        prepare_registry();
    }
    void TearDown(const benchmark::State& /*state*/) override {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
    }
};

BENCHMARK_DEFINE_F(register_scaling_fixture_t, register_plugins)(benchmark::State& state) {
    const size_t count = static_cast<size_t>(state.range(0));
    auto& registry = plugin_registry_t::instance();
    for (auto _ : state) {
        registry.clear();
        std::vector<std::unique_ptr<null_plugin_t>> plugins;
        plugins.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            plugins.push_back(std::make_unique<null_plugin_t>("reg_" + std::to_string(i)));
            registry.register_plugin_ref(*plugins.back());
        }
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    state.SetLabel("register " + std::to_string(count) + " plugins");
}
BENCHMARK_REGISTER_F(register_scaling_fixture_t, register_plugins)
    ->Arg(1)->Arg(4)->Arg(16)->Arg(64)
    ->MinTime(0.5);

BENCHMARK_DEFINE_F(register_scaling_fixture_t, unregister_plugins)(benchmark::State& state) {
    const size_t count = static_cast<size_t>(state.range(0));
    auto& registry = plugin_registry_t::instance();
    for (auto _ : state) {
        std::vector<std::unique_ptr<null_plugin_t>> plugins;
        plugins.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            plugins.push_back(std::make_unique<null_plugin_t>("unreg_" + std::to_string(i)));
            registry.register_plugin_ref(*plugins.back());
        }
        for (size_t i = 0; i < count; ++i) {
            registry.unregister_plugin(plugins[i]->name());
        }
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(count));
    state.SetLabel("unregister " + std::to_string(count) + " plugins");
}
BENCHMARK_REGISTER_F(register_scaling_fixture_t, unregister_plugins)
    ->Arg(1)->Arg(4)->Arg(16)->Arg(64)
    ->MinTime(0.5);

BENCHMARK_MAIN();
