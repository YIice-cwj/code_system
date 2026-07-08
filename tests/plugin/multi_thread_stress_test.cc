#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "error_system/config/feature_flags.h"
#include "error_system/core/error_context.h"
#include "error_system/core/i_error_notifier.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/plugin/plugin_registry.h"

/**
 * @file multi_thread_stress_test.cc
 * @brief 多线程 API 压力测试：死锁与数据竞争检测
 * @details 对 plugin_registry / error_context / error_registry / i_error_notifier
 *          的并发接口施加多线程压力，验证不死锁、不崩溃、计数正确。
 *          死锁判定方式：join() 返回即未死锁；若死锁则测试 hang 直至 CI 超时。
 *          TSan 模式下运行可检测数据竞争。
 * @author yiice
 * @version 4.3.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::plugin {
    namespace {
        using core::error_code_t;
        using core::error_context_t;
        using core::error_level_t;
        using core::error_number_t;
        using core::module_id_t;
        using core::subsystem_id_t;
        using domain::system_domain_t;

        constexpr int THREAD_COUNT = 8;
        constexpr int ITERATIONS = 500;

        error_code_t make_stress_code(uint16_t number) noexcept {
            return error_code_t(error_level_t::error, system_domain_t::application,
                                 subsystem_id_t{1}, module_id_t{1}, error_number_t{number});
        }

        class counting_plugin_t : public i_error_plugin_t {
            public:
            explicit counting_plugin_t(std::string n) : plugin_name(std::move(n)) {}
            std::string_view name() const noexcept override { return plugin_name; }
            void on_error(const core::error_context_t&) noexcept override {
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
            void on_code(core::error_code_t) noexcept override {
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
            std::string plugin_name;
            std::atomic<int> call_count{0};
        };

        void wait_for_pending(size_t expected_max, int timeout_ms) noexcept {
            const auto deadline = std::chrono::steady_clock::now()
                                + std::chrono::milliseconds(timeout_ms);
            while (std::chrono::steady_clock::now() < deadline) {
                if (plugin_registry_t::instance().pending_notifications() <= expected_max) {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    }  // namespace

    class multi_thread_stress_test_t : public ::testing::Test {
        protected:
        void SetUp() override {
            core::error_registry_t::instance().unregister_all();
            plugin_registry_t::instance().clear();
            config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync);
#ifdef ERROR_SYSTEM_ENABLE_STACKTRACE
            config::feature_flags_t::set_enable_stacktrace(false);
#endif
#ifdef ERROR_SYSTEM_ENABLE_VALIDATION
            config::feature_flags_t::set_enable_validation(false);
#endif
            const auto code = make_stress_code(1);
            core::error_registry_t::instance().register_error(code, "STRESS_1", "stress");
        }

        void TearDown() override {
            config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync);
            wait_for_pending(0, 2000);
            plugin_registry_t::instance().clear();
            core::error_registry_t::instance().unregister_all();
        }
    };

    TEST_F(multi_thread_stress_test_t, concurrent_notify_error_with_stable_registry_no_deadlock) {
        counting_plugin_t plugin("stress_counter");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        const auto code = make_stress_code(1);
        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        for (int t = 0; t < THREAD_COUNT; ++t) {
            threads.emplace_back([code]() {
                for (int i = 0; i < ITERATIONS; ++i) {
                    error_context_t ctx(core::located_code_t{code}, "stress");
                    plugin_registry_t::instance().notify_error(ctx);
                }
            });
        }
        for (auto& th : threads) { th.join(); }

        EXPECT_EQ(plugin.call_count.load(), THREAD_COUNT * ITERATIONS);
    }

    TEST_F(multi_thread_stress_test_t, concurrent_notify_with_register_unregister_no_deadlock) {
        counting_plugin_t plugin("survivor");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        const auto code = make_stress_code(1);
        std::atomic<bool> start{false};

        std::thread notifier([&]() {
            while (!start.load()) { std::this_thread::yield(); }
            for (int i = 0; i < ITERATIONS; ++i) {
                error_context_t ctx(core::located_code_t{code}, "concurrent");
                plugin_registry_t::instance().notify_error(ctx);
            }
        });

        std::thread churner([&]() {
            while (!start.load()) { std::this_thread::yield(); }
            for (int i = 0; i < 100; ++i) {
                plugin_registry_t::instance().unregister_plugin("survivor");
                plugin_registry_t::instance().register_plugin_ref(plugin);
            }
        });

        start.store(true);
        notifier.join();
        churner.join();

        EXPECT_GT(plugin.call_count.load(), 0);
    }

    TEST_F(multi_thread_stress_test_t, concurrent_lean_notify_via_try_notify_no_deadlock) {
        config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::async_queue);
        config::feature_flags_t::set_async_queue_max_size(0);

        auto plugin_owner = std::make_unique<counting_plugin_t>("lean_async");
        counting_plugin_t* plugin = plugin_owner.get();
        plugin_registry_t::instance().register_plugin(std::move(plugin_owner));

        const auto code = make_stress_code(1);
        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        for (int t = 0; t < THREAD_COUNT; ++t) {
            threads.emplace_back([code]() {
                for (int i = 0; i < ITERATIONS; ++i) {
                    core::i_error_notifier_t::try_notify(code);
                }
            });
        }
        for (auto& th : threads) { th.join(); }

        wait_for_pending(0, 5000);
        EXPECT_GT(plugin->call_count.load(), 0);
    }

    TEST_F(multi_thread_stress_test_t, concurrent_deferred_notify_flush_per_thread_no_deadlock) {
        config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync_deferred);
        config::feature_flags_t::set_deferred_buffer_max_size(0);

        counting_plugin_t plugin("deferred");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        const auto code = make_stress_code(1);
        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        for (int t = 0; t < THREAD_COUNT; ++t) {
            threads.emplace_back([code]() {
                for (int i = 0; i < ITERATIONS; ++i) {
                    error_context_t ctx(core::located_code_t{code}, "deferred");
                    plugin_registry_t::instance().notify(ctx);
                }
                plugin_registry_t::instance().flush_deferred_notifications();
            });
        }
        for (auto& th : threads) { th.join(); }

        EXPECT_EQ(plugin.call_count.load(), THREAD_COUNT * ITERATIONS);
    }

    TEST_F(multi_thread_stress_test_t, concurrent_error_context_clone_no_deadlock) {
        const auto code = make_stress_code(1);
        error_context_t source(core::located_code_t{code}, "clone source");
        source.with("key", "value");

        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        std::atomic<int> success_count{0};
        for (int t = 0; t < THREAD_COUNT; ++t) {
            threads.emplace_back([&source, &success_count]() {
                for (int i = 0; i < ITERATIONS; ++i) {
                    auto copy = source.clone();
                    if (copy.get_code().get_code() == source.get_code().get_code()) {
                        success_count.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }
        for (auto& th : threads) { th.join(); }

        EXPECT_EQ(success_count.load(), THREAD_COUNT * ITERATIONS);
    }

    TEST_F(multi_thread_stress_test_t, concurrent_registry_read_write_no_deadlock) {
        std::vector<std::thread> threads;
        threads.reserve(THREAD_COUNT);
        std::atomic<int> hit_count{0};

        for (int t = 0; t < THREAD_COUNT; ++t) {
            threads.emplace_back([t, &hit_count]() {
                const uint16_t base = static_cast<uint16_t>(t * 10 + 100);
                for (int i = 0; i < ITERATIONS; ++i) {
                    const auto code = make_stress_code(static_cast<uint16_t>(base + (i % 10)));
                    if (i % 5 == 0) {
                        core::error_registry_t::instance().register_error(
                            code, "CONC_REG", "concurrent register");
                    }
                    if (core::error_registry_t::instance().get_info_cached(code).has_value()) {
                        hit_count.fetch_add(1, std::memory_order_relaxed);
                    }
                }
            });
        }
        for (auto& th : threads) { th.join(); }

        EXPECT_GT(hit_count.load(), 0);
    }

    TEST_F(multi_thread_stress_test_t, mixed_sync_and_async_notify_no_deadlock) {
        auto plugin_owner = std::make_unique<counting_plugin_t>("mixed");
        counting_plugin_t* plugin = plugin_owner.get();
        plugin_registry_t::instance().register_plugin(std::move(plugin_owner));

        const auto code = make_stress_code(1);
        std::atomic<bool> stop{false};

        std::thread async_worker([&]() {
            while (!stop.load()) {
                error_context_t ctx(core::located_code_t{code}, "async");
                plugin_registry_t::instance().enqueue_notification(ctx);
                std::this_thread::yield();
            }
        });

        std::thread sync_worker([&]() {
            while (!stop.load()) {
                error_context_t ctx(core::located_code_t{code}, "sync");
                plugin_registry_t::instance().notify_error(ctx);
                std::this_thread::yield();
            }
        });

        std::thread mode_switcher([&]() {
            for (int i = 0; i < 50; ++i) {
                const auto mode = (i % 2 == 0)
                    ? config::feature_flags_t::notify_mode_t::sync
                    : config::feature_flags_t::notify_mode_t::async_queue;
                config::feature_flags_t::set_notify_mode(mode);
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
        });

        mode_switcher.join();
        stop.store(true);
        async_worker.join();
        sync_worker.join();

        wait_for_pending(0, 5000);
        EXPECT_GT(plugin->call_count.load(), 0);
    }

}  // namespace error_system::plugin
