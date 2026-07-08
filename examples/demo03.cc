/**
 * @file demo03.cc
 * @brief 插件系统 API 全览
 * @details 演示 plugin_registry_t、i_error_plugin_t、error_router_plugin_t、
 *          error_dedup_sampler_t、通知模式（sync/async/sync_deferred）的全部公共 API。
 *          每个示例独立成函数，标题即 API 名。
 */

#include <atomic>
#include <iostream>
#include <unordered_map>

#include "error_system.h"
#include "error_system/plugin/error_dedup_sampler.h"
// IWYU pragma: begin_exports
#include "payment_service_errors.h"
#include "redis_component_errors.h"
#include "trade_service_errors.h"
#include "user_service_errors.h"
// IWYU pragma: end_exports

using namespace error_system::core;
using namespace error_system::plugin;
using namespace error_system::config;
using namespace error_system::domain;

namespace {

/** @brief 打印小节标题 */
void section(const std::string& title) {
    std::cout << "\n--- " << title << " ---" << std::endl;
}

/** @brief 自定义日志插件：打印所有级别错误 */
class demo_log_plugin_t : public i_error_plugin_t {
public:
    std::string_view name() const noexcept override { return "logger"; }

    error_level_t min_level() const noexcept override {
        return error_level_t::debug;
    }

    void on_error(const error_context_t& context) noexcept override {
        std::cerr << "[LOG] " << context.get_message() << std::endl;
    }

    void on_code(error_code_t code) noexcept override {
        std::cerr << "[LOG] [ERR: " << code.get_code() << "]" << std::endl;
    }
};

/** @brief 统计插件：用 atomic 计数各错误码出现次数 */
class stats_plugin_t : public i_error_plugin_t {
    std::unordered_map<uint64_t, std::atomic<int>> counters_;

public:
    std::string_view name() const noexcept override { return "stats"; }

    error_level_t min_level() const noexcept override {
        return error_level_t::error;
    }

    void on_error(const error_context_t& context) noexcept override {
        try {
            ++counters_[context.get_code().get_code()];
        } catch (const std::bad_alloc&) {
        }
    }

    void on_code(error_code_t code) noexcept override {
        try {
            ++counters_[code.get_code()];
        } catch (const std::bad_alloc&) {
        }
    }

    int total() const noexcept {
        int sum = 0;
        for (const auto& [_, c] : counters_) { sum += c.load(); }
        return sum;
    }
};

demo_log_plugin_t g_logger;
stats_plugin_t g_stats;

/** @brief 1.1 register_plugin_ref 注册引用（栈对象） */
void demo_register_plugin_ref() {
    section("1.1 register_plugin_ref 注册引用（栈对象）");
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    registry.register_plugin_ref(g_logger);
    std::cout << "  插件数: " << registry.size() << std::endl;
}

/** @brief 1.2 register_plugin 注册 unique_ptr（堆对象） */
void demo_register_plugin_unique_ptr() {
    section("1.2 register_plugin 注册 unique_ptr（堆对象）");
    auto& registry = plugin_registry_t::instance();
    auto stats_ptr = std::make_unique<stats_plugin_t>();
    stats_plugin_t* stats_ref = stats_ptr.get();
    registry.register_plugin(std::move(stats_ptr));
    std::cout << "  插件数: " << registry.size() << std::endl;
    (void)stats_ref;
}

/** @brief 1.3 min_level 级别过滤效果说明 */
void demo_min_level_filter() {
    section("1.3 min_level 级别过滤效果");
    std::cout << "  logger min_level=debug (所有级别)" << std::endl;
    std::cout << "  stats  min_level=error (error+fatal)" << std::endl;
}

/** @brief 2.1 size() 插件数量 */
void demo_size() {
    section("2.1 size() 插件数量");
    auto& registry = plugin_registry_t::instance();
    std::cout << "  " << registry.size() << std::endl;
}

/** @brief 2.2 empty() 是否为空 */
void demo_empty() {
    section("2.2 empty() 是否为空");
    auto& registry = plugin_registry_t::instance();
    std::cout << "  empty = " << registry.empty() << std::endl;
}

/** @brief 2.3 is_initialized() 是否已初始化 */
void demo_is_initialized() {
    section("2.3 is_initialized() 是否已初始化");
    std::cout << "  is_initialized = " << plugin_registry_t::is_initialized() << std::endl;
}

/** @brief 3.1 触发 error 级别（logger + stats 都收到） */
void demo_notify_error_level() {
    section("3.1 触发 error 级别（logger + stats 都收到）");
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    registry.register_plugin_ref(g_logger);
    auto stats_ptr = std::make_unique<stats_plugin_t>();
    stats_plugin_t* stats_ref = stats_ptr.get();
    registry.register_plugin(std::move(stats_ptr));
    error_context_t ctx_err{biz::trade_errors::ERR_ORDER_NOT_FOUND, "订单错误"};
    std::cout << "  stats 统计总数: " << stats_ref->total() << std::endl;
    (void)ctx_err;
}

/** @brief 3.2 触发 info 级别（仅 logger 收到，stats 过滤） */
void demo_notify_info_level() {
    section("3.2 触发 info 级别（仅 logger 收到，stats 过滤）");
    auto stats_ptr = std::make_unique<stats_plugin_t>();
    stats_plugin_t* stats_ref = stats_ptr.get();
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    registry.register_plugin_ref(g_logger);
    registry.register_plugin(std::move(stats_ptr));
    error_context_t ctx_info{biz::user_errors::ERR_TOKEN_EXPIRED, "Token 过期"};
    std::cout << "  stats 统计总数: " << stats_ref->total() << " (应不变)" << std::endl;
    (void)ctx_info;
}

/** @brief 3.3 Lean 路径：result_t<T, true> 触发 on_code 而非 on_error */
void demo_lean_on_code() {
    section("3.3 Lean 路径：result_t<T, true> 触发 on_code");
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    registry.register_plugin_ref(g_logger);
    auto r = result_t<int, true>::make_error(biz::trade_errors::ERR_ORDER_NOT_FOUND, "Lean 错误");
    std::cout << "  is_error=" << r.is_error() << " (上方应出现 [LOG] [ERR: ...])" << std::endl;
}

/** @brief 4.1 unregister_plugin 按名称注销 */
void demo_unregister_plugin() {
    section("4.1 unregister_plugin 按名称注销");
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    registry.register_plugin_ref(g_logger);
    std::cout << "  注销前插件数: " << registry.size() << std::endl;
    registry.unregister_plugin("logger");
    std::cout << "  注销后插件数: " << registry.size() << std::endl;
}

/** @brief 4.2 clear 清空所有插件 */
void demo_clear() {
    section("4.2 clear 清空所有插件");
    auto& registry = plugin_registry_t::instance();
    registry.register_plugin_ref(g_logger);
    std::cout << "  清空前插件数: " << registry.size() << std::endl;
    registry.clear();
    std::cout << "  清空后插件数: " << registry.size() << std::endl;
}

/** @brief 5.1 error_router_plugin_t 按错误码路由 */
void demo_router_by_code() {
    section("5.1 error_router_plugin_t 按错误码路由");
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    error_router_plugin_t::instance().register_handler_by_code(
        biz::trade_errors::ERR_ORDER_NOT_FOUND,
        [](const error_context_t& ctx) {
            std::cout << "  [路由] 订单错误: " << ctx.get_message() << std::endl;
        });
    registry.register_plugin_ref(error_router_plugin_t::instance());
    error_context_t ctx_route{biz::trade_errors::ERR_ORDER_NOT_FOUND, "路由测试"};
    std::cout << "  (上方应出现路由输出)" << std::endl;
    (void)ctx_route;
}

/** @brief 5.2 register_handler_by_domain 按域路由 */
void demo_router_by_domain() {
    section("5.2 register_handler_by_domain 按域路由");
    error_router_plugin_t::instance().register_handler_by_domain(
        system_domain_t::middleware,
        [](const error_context_t& ctx) {
            std::cout << "  [中间件域] " << ctx.get_message() << std::endl;
        });
    error_context_t ctx_mid{infra::redis_errors::ERR_POOL_EXHAUSTED, "Redis 故障"};
    std::cout << "  (上方应出现中间件域输出)" << std::endl;
    (void)ctx_mid;
}

/** @brief 5.3 unregister_handler_by_code 注销路由 */
void demo_router_unregister() {
    section("5.3 unregister_handler_by_code 注销路由");
    auto& registry = plugin_registry_t::instance();
    error_router_plugin_t::instance().unregister_handler_by_code(biz::trade_errors::ERR_ORDER_NOT_FOUND);
    error_context_t ctx_unroute{biz::trade_errors::ERR_ORDER_NOT_FOUND, "不再路由"};
    std::cout << "  (上方不应出现路由输出)" << std::endl;
    registry.unregister_plugin("router");
    error_router_plugin_t::instance().unregister_handler_by_domain(system_domain_t::middleware);
    (void)ctx_unroute;
}

/** @brief 5.4 error_router_plugin_t Lean 路径：register_code_handler_by_code */
void demo_router_code_handler() {
    section("5.4 error_router_plugin_t Lean 路径：register_code_handler_by_code");
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    error_router_plugin_t::instance().register_code_handler_by_code(
        biz::trade_errors::ERR_ORDER_NOT_FOUND,
        [](error_code_t code) {
            std::cout << "  [Lean 路由] code=" << code.get_code() << std::endl;
        });
    registry.register_plugin_ref(error_router_plugin_t::instance());
    auto r = result_t<int, true>::make_error(biz::trade_errors::ERR_ORDER_NOT_FOUND, "Lean 路由");
    error_router_plugin_t::instance().unregister_code_handler_by_code(biz::trade_errors::ERR_ORDER_NOT_FOUND);
    std::cout << "  is_error=" << r.is_error() << " (上方应出现 [Lean 路由] 输出)" << std::endl;
}

/** @brief 6.1 sync 同步模式（默认） */
void demo_notify_sync() {
    section("6.1 sync 同步模式（默认）");
    auto& registry = plugin_registry_t::instance();
    registry.clear();
    registry.register_plugin_ref(g_logger);
    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    std::cout << "  触发错误时立即通知" << std::endl;
    error_context_t ctx_sync{biz::trade_errors::ERR_ORDER_NOT_FOUND, "同步通知"};
    (void)ctx_sync;
}

/** @brief 6.2 async_queue 异步模式 + set_max_queue_size(100) 背压 */
void demo_notify_async() {
    section("6.2 async_queue 异步模式 + set_max_queue_size(100) 背压");
    auto& registry = plugin_registry_t::instance();
    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::async_queue);
    registry.set_max_queue_size(100);
    constexpr int ASYNC_TOTAL = 200;
    for (int i = 0; i < ASYNC_TOTAL; ++i) {
        error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "异步" + std::to_string(i)};
        (void)ctx;
    }
    std::cout << "  max_queue_size = " << registry.get_max_queue_size() << std::endl;
    std::cout << "  入队 " << ASYNC_TOTAL << " 条后 pending = " << registry.pending_notifications()
              << " (工作线程并发消费，pending 为瞬时值)" << std::endl;
    registry.clear();
}

/** @brief 6.3 sync_deferred 延迟模式 + set_deferred_buffer_size(100) */
void demo_notify_deferred() {
    section("6.3 sync_deferred 延迟模式 + set_deferred_buffer_size(100)");
    auto& registry = plugin_registry_t::instance();
    registry.register_plugin_ref(g_logger);
    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
    registry.set_deferred_buffer_size(100);
    std::cout << "  buffer_size = " << registry.get_deferred_buffer_size() << std::endl;
}

/** @brief 6.4 批量入队 200 条验证缓冲溢出（不触发消费） */
void demo_deferred_overflow() {
    section("6.4 批量入队 200 条验证缓冲溢出（不触发消费）");
    auto& registry = plugin_registry_t::instance();
    constexpr int DEFER_TOTAL = 200;
    for (int i = 0; i < DEFER_TOTAL; ++i) {
        error_context_t d{biz::trade_errors::ERR_ORDER_NOT_FOUND, "延迟" + std::to_string(i)};
        (void)d;
    }
    std::cout << "  预期 pending=100 丢弃=" << (DEFER_TOTAL - 100) << std::endl;
    std::cout << "  实际 pending=" << registry.pending_deferred_notifications() << std::endl;
}

/** @brief 6.5 deferred_buffer_overflowed 缓冲溢出标志 */
void demo_deferred_overflowed_flag() {
    section("6.5 deferred_buffer_overflowed 缓冲溢出标志");
    auto& registry = plugin_registry_t::instance();
    std::cout << "  预期 overflowed=1" << std::endl;
    std::cout << "  实际 overflowed = " << registry.deferred_buffer_overflowed() << std::endl;
}

/** @brief 6.6 flush_deferred_notifications 批量通知 */
void demo_flush_deferred() {
    section("6.6 flush_deferred_notifications 批量通知");
    auto& registry = plugin_registry_t::instance();
    registry.flush_deferred_notifications();
    std::cout << "  flush 后 pending = " << registry.pending_deferred_notifications() << std::endl;
}

/** @brief 6.7 clear_deferred_notifications 清空缓冲 */
void demo_clear_deferred() {
    section("6.7 clear_deferred_notifications 清空缓冲");
    auto& registry = plugin_registry_t::instance();
    error_context_t d3{biz::trade_errors::ERR_ORDER_NOT_FOUND, "延迟3"};
    auto dropped = registry.clear_deferred_notifications();
    std::cout << "  清空丢弃数 = " << dropped << std::endl;
    (void)d3;
    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    registry.clear();
}

/** @brief 7.1 去重窗口验证：同错误码循环 100 次（rate=1.0 仅去重） */
void demo_dedup_window() {
    section("7.1 去重窗口验证：同错误码循环 100 次（rate=1.0 仅去重）");
    error_dedup_sampler_t sampler;
    sampler.set_dedup_window_ms(1000);
    sampler.set_sample_rate(1.0);
    sampler.reset_stats();
    sampler.clear_dedup_cache();
    error_context_t sample_ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "去重测试"};
    constexpr int DEDUP_TOTAL = 100;
    int local_forwarded = 0;
    for (int i = 0; i < DEDUP_TOTAL; ++i) {
        if (sampler.should_be_forwarded(sample_ctx)) { ++local_forwarded; }
    }
    std::cout << "  预期 forwarded=1  deduped=" << (DEDUP_TOTAL - 1) << std::endl;
    std::cout << "  实际 forwarded=" << sampler.forwarded_count()
              << "  deduped=" << sampler.deduped_count()
              << "  sampled=" << sampler.sampled_count() << std::endl;
    std::cout << "  本地计数 forwarded=" << local_forwarded
              << " (与采样器统计一致)" << std::endl;
}

/** @brief 7.2 采样率验证：不同错误码循环 1000 次（rate=0.5 window=0） */
void demo_sample_rate() {
    section("7.2 采样率验证：不同错误码循环 1000 次（rate=0.5 window=0）");
    error_dedup_sampler_t sampler;
    sampler.set_dedup_window_ms(0);
    sampler.set_sample_rate(0.5);
    sampler.reset_stats();
    sampler.clear_dedup_cache();
    constexpr int SAMPLE_TOTAL = 1000;
    int local_sample_forwarded = 0;
    for (int i = 0; i < SAMPLE_TOTAL; ++i) {
        error_code_t code{error_level_t::error,
                          system_domain_t::application,
                          subsystem_id_t{1},
                          module_id_t{1},
                          error_number_t{static_cast<uint16_t>(i + 1)}};
        error_context_t ctx{code, "采样测试"};
        if (sampler.should_be_forwarded(ctx)) { ++local_sample_forwarded; }
    }
    const int expected_forward = SAMPLE_TOTAL / 2;
    const int expected_sampled = SAMPLE_TOTAL - expected_forward;
    std::cout << "  预期 forwarded=" << expected_forward
              << "  sampled=" << expected_sampled << std::endl;
    std::cout << "  实际 forwarded=" << sampler.forwarded_count()
              << "  sampled=" << sampler.sampled_count()
              << "  deduped=" << sampler.deduped_count() << std::endl;
    std::cout << "  本地计数 forwarded=" << local_sample_forwarded
              << " (与采样器统计一致)" << std::endl;
}

/** @brief 7.3 deduped_count / sampled_count / forwarded_count 统计查询 */
void demo_sampler_stats() {
    section("7.3 deduped_count / sampled_count / forwarded_count 统计查询");
    error_dedup_sampler_t sampler;
    sampler.set_dedup_window_ms(0);
    sampler.set_sample_rate(1.0);
    sampler.reset_stats();
    sampler.clear_dedup_cache();
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "统计测试"};
    [[maybe_unused]] const bool forwarded = sampler.should_be_forwarded(ctx);
    std::cout << "  deduped  = " << sampler.deduped_count() << std::endl;
    std::cout << "  sampled  = " << sampler.sampled_count() << std::endl;
    std::cout << "  forwarded= " << sampler.forwarded_count() << std::endl;
}

/** @brief 7.4 reset_stats 重置统计 */
void demo_sampler_reset() {
    section("7.4 reset_stats 重置统计");
    error_dedup_sampler_t sampler;
    sampler.set_sample_rate(1.0);
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "重置前"};
    [[maybe_unused]] const bool forwarded = sampler.should_be_forwarded(ctx);
    sampler.reset_stats();
    std::cout << "  重置后 forwarded = " << sampler.forwarded_count() << std::endl;
}

/** @brief 7.5 clear_dedup_cache 清空去重缓存 */
void demo_sampler_clear_cache() {
    section("7.5 clear_dedup_cache 清空去重缓存");
    error_dedup_sampler_t sampler;
    sampler.set_dedup_window_ms(1000);
    sampler.set_sample_rate(1.0);
    error_context_t ctx{biz::trade_errors::ERR_ORDER_NOT_FOUND, "清空前"};
    [[maybe_unused]] const bool forwarded = sampler.should_be_forwarded(ctx);
    sampler.clear_dedup_cache();
    std::cout << "  缓存已清空" << std::endl;
}

}  // namespace

int main() {
    std::cout << "===== Demo 3: 插件系统 API 全览 =====" << std::endl;

    demo_register_plugin_ref();
    demo_register_plugin_unique_ptr();
    demo_min_level_filter();

    demo_size();
    demo_empty();
    demo_is_initialized();

    demo_notify_error_level();
    demo_notify_info_level();
    demo_lean_on_code();

    demo_unregister_plugin();
    demo_clear();

    demo_router_by_code();
    demo_router_by_domain();
    demo_router_unregister();
    demo_router_code_handler();

    demo_notify_sync();
    demo_notify_async();
    demo_notify_deferred();
    demo_deferred_overflow();
    demo_deferred_overflowed_flag();
    demo_flush_deferred();
    demo_clear_deferred();

    demo_dedup_window();
    demo_sample_rate();
    demo_sampler_stats();
    demo_sampler_reset();
    demo_sampler_clear_cache();

    return 0;
}
