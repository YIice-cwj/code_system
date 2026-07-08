#include "error_system/plugin/plugin_registry.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "error_system/config/feature_flags.h"
#include "error_system/core/error_context.h"

namespace error_system::plugin {

    using error_system::core::error_code_t;

    class mock_plugin_t : public i_error_plugin_t {
        public:
        std::string plugin_name;
        mutable std::atomic<int> call_count{0};
        mutable std::atomic<const core::error_context_t*> last_context{nullptr};

        explicit mock_plugin_t(const std::string& name) : plugin_name(name) {}

        std::string_view name() const noexcept override { return plugin_name; }

        void on_error(const core::error_context_t& context) noexcept override {
            call_count.fetch_add(1);
            last_context.store(&context, std::memory_order_relaxed);
        }
    };

    class plugin_registry_test_t : public ::testing::Test {
        protected:
        void SetUp() override {
            plugin_registry_t::instance().clear();
            error_system::core::error_registry_t::instance().register_error(
                error_system::core::error_code_t(0x800000000000002AULL), "TEST_CODE_42", "test");
        }

        void TearDown() override { plugin_registry_t::instance().clear(); }
    };

    TEST_F(plugin_registry_test_t, register_plugin_increases_size) {
        mock_plugin_t plugin1("plugin1");
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);

        plugin_registry_t::instance().register_plugin_ref(plugin1);
        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);
    }

    TEST_F(plugin_registry_test_t, empty_returns_true_when_no_plugins) {
        EXPECT_TRUE(plugin_registry_t::instance().empty());

        mock_plugin_t plugin("test");
        plugin_registry_t::instance().register_plugin_ref(plugin);
        EXPECT_FALSE(plugin_registry_t::instance().empty());
    }

    TEST_F(plugin_registry_test_t, register_duplicate_replaces_old) {
        mock_plugin_t plugin1("same_name");
        mock_plugin_t plugin2("same_name");

        plugin_registry_t::instance().register_plugin_ref(plugin1);
        plugin_registry_t::instance().register_plugin_ref(plugin2);

        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);
    }

    TEST_F(plugin_registry_test_t, unregister_plugin_removes_it) {
        mock_plugin_t plugin("to_remove");
        plugin_registry_t::instance().register_plugin_ref(plugin);
        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);

        plugin_registry_t::instance().unregister_plugin("to_remove");
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);
    }

    TEST_F(plugin_registry_test_t, unregister_nonexistent_does_nothing) {
        mock_plugin_t plugin("exists");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        plugin_registry_t::instance().unregister_plugin("nonexistent");
        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);
    }

    TEST_F(plugin_registry_test_t, notify_error_calls_all_plugins) {
        mock_plugin_t plugin1("p1");
        mock_plugin_t plugin2("p2");

        plugin_registry_t::instance().register_plugin_ref(plugin1);
        plugin_registry_t::instance().register_plugin_ref(plugin2);

        core::error_context_t context;
        plugin_registry_t::instance().notify_error(context);

        EXPECT_EQ(plugin1.call_count.load(), 1);
        EXPECT_EQ(plugin2.call_count.load(), 1);
    }

    TEST_F(plugin_registry_test_t, clear_removes_all_plugins) {
        mock_plugin_t plugin1("p1");
        mock_plugin_t plugin2("p2");

        plugin_registry_t::instance().register_plugin_ref(plugin1);
        plugin_registry_t::instance().register_plugin_ref(plugin2);
        EXPECT_EQ(plugin_registry_t::instance().size(), 2UL);

        plugin_registry_t::instance().clear();
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);
        EXPECT_TRUE(plugin_registry_t::instance().empty());
    }

    TEST_F(plugin_registry_test_t, singleton_returns_same_instance) {
        auto& instance1 = plugin_registry_t::instance();
        auto& instance2 = plugin_registry_t::instance();
        EXPECT_EQ(&instance1, &instance2);
    }

    TEST_F(plugin_registry_test_t, notify_error_passes_correct_context) {
        mock_plugin_t plugin("test");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        core::error_context_t context(core::located_code_t{core::error_code_t(0x800000000000002AULL)}, "test message");
        plugin_registry_t::instance().notify_error(context);

        EXPECT_EQ(plugin.last_context.load(std::memory_order_relaxed)->get_code().get_code(), 0x800000000000002AULL);
    }

    TEST_F(plugin_registry_test_t, concurrent_notify_with_stable_registry) {
        mock_plugin_t plugin("concurrent");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        std::vector<std::thread> threads;
        std::atomic<int> notify_count{0};

        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&notify_count]() {
                for (int j = 0; j < 100; ++j) {
                    core::error_context_t context(core::located_code_t{core::error_code_t(0x800000000000002AULL)}, "test");
                    plugin_registry_t::instance().notify_error(context);
                    notify_count.fetch_add(1);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        EXPECT_EQ(notify_count.load(), 1000);
        EXPECT_EQ(plugin.call_count.load(), 1000);
    }

    // 冒烟级别测试：各线程操作不相交插件，仅验证并发期间不崩溃
    TEST_F(plugin_registry_test_t, concurrent_register_and_unregister_smoke) {
        std::vector<std::unique_ptr<mock_plugin_t>> plugins;
        for (int i = 0; i < 100; ++i) {
            plugins.push_back(std::make_unique<mock_plugin_t>("plugin_" + std::to_string(i)));
        }

        std::vector<std::thread> threads;

        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([&plugins, i]() {
                for (int j = 0; j < 10; ++j) {
                    int idx = i * 10 + j;
                    plugin_registry_t::instance().register_plugin_ref(*plugins[static_cast<size_t>(idx)]);
                    plugin_registry_t::instance().unregister_plugin("plugin_" + std::to_string(idx));
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        // 冒烟验证：所有插件已注销，并发期间未崩溃
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);
    }

    TEST_F(plugin_registry_test_t, concurrent_unregister_during_notify_does_not_deadlock) {
        mock_plugin_t plugin("target");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        std::atomic<bool> notification_started{false};
        std::atomic<bool> unregister_done{false};

        std::thread notifier([&notification_started]() {
            notification_started.store(true);
            for (int i = 0; i < 2000; ++i) {
                core::error_context_t context(core::located_code_t{core::error_code_t(0x800000000000002AULL)}, "stress");
                plugin_registry_t::instance().notify_error(context);
            }
        });

        while (!notification_started.load()) {
            std::this_thread::yield();
        }

        std::thread unregisterer([&unregister_done, &plugin]() {
            for (int i = 0; i < 500; ++i) {
                plugin_registry_t::instance().unregister_plugin("target");
                plugin_registry_t::instance().register_plugin_ref(plugin);
            }
            unregister_done.store(true);
        });

        notifier.join();
        unregisterer.join();

        EXPECT_TRUE(unregister_done.load());
        EXPECT_GT(plugin.call_count.load(), 500L);
        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);
    }

    TEST_F(plugin_registry_test_t, slow_plugin_does_not_cause_deadlock) {
        class slow_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "slow_plugin"; }
            void on_error(const core::error_context_t&) noexcept override {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait_for(lock, std::chrono::microseconds(100));
                call_count.fetch_add(1);
            }
            std::atomic<int> call_count{0};
            std::mutex mutex{};
            std::condition_variable cv{};
        };

        slow_plugin_t slow_plugin;
        mock_plugin_t normal_plugin("normal");
        std::atomic<int> registrant_ops{0};

        plugin_registry_t::instance().register_plugin_ref(slow_plugin);

        std::thread notifier([]() {
            for (int i = 0; i < 30; ++i) {
                core::error_context_t context(core::located_code_t{core::error_code_t(0x800000000000002AULL)}, "test");
                plugin_registry_t::instance().notify_error(context);
            }
        });

        std::thread registrant([&normal_plugin, &registrant_ops]() {
            for (int i = 0; i < 50; ++i) {
                plugin_registry_t::instance().register_plugin_ref(normal_plugin);
                plugin_registry_t::instance().unregister_plugin("normal");
                registrant_ops.fetch_add(1);
            }
        });

        notifier.join();
        registrant.join();

        // 两线程均完成（join 未阻塞说明无死锁）
        EXPECT_GT(slow_plugin.call_count.load(), 0);
        EXPECT_EQ(registrant_ops.load(), 50);
    }

    TEST_F(plugin_registry_test_t, register_plugin_nullptr) {
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);
        plugin_registry_t::instance().register_plugin(std::unique_ptr<i_error_plugin_t>{});
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);
    }

    TEST_F(plugin_registry_test_t, register_plugin_ownership_transfer) {
        auto plugin = std::make_unique<mock_plugin_t>("owned_plugin");
        plugin_registry_t::instance().register_plugin(std::move(plugin));
        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);

        plugin_registry_t::instance().unregister_plugin("owned_plugin");
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);
    }

    TEST_F(plugin_registry_test_t, register_plugin_ownership_replace_old) {
        auto plugin1 = std::make_unique<mock_plugin_t>("owned_replace");
        plugin_registry_t::instance().register_plugin(std::move(plugin1));
        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);

        auto plugin2 = std::make_unique<mock_plugin_t>("owned_replace");
        plugin_registry_t::instance().register_plugin(std::move(plugin2));
        EXPECT_EQ(plugin_registry_t::instance().size(), 1UL);

        plugin_registry_t::instance().clear();
        EXPECT_EQ(plugin_registry_t::instance().size(), 0UL);
    }

    TEST_F(plugin_registry_test_t, min_level_filtering) {
        class level_filter_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "level_filter"; }
            core::error_level_t min_level() const noexcept override { return core::error_level_t::error; }
            void on_error(const core::error_context_t&) noexcept override {
                call_count.fetch_add(1);
            }
            std::atomic<int> call_count{0};
        };

        level_filter_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);

        error_system::core::error_registry_t::instance().register_error(
            error_code_t(core::error_level_t::debug, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{1}),
            "TEST_DEBUG_1", "debug test");
        error_system::core::error_registry_t::instance().register_error(
            error_code_t(core::error_level_t::info, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{2}),
            "TEST_INFO_1", "info test");
        error_system::core::error_registry_t::instance().register_error(
            error_code_t(core::error_level_t::warn, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{3}),
            "TEST_WARN_1", "warn test");
        error_system::core::error_registry_t::instance().register_error(
            error_code_t(core::error_level_t::error, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{4}),
            "TEST_ERROR_1", "error test");
        error_system::core::error_registry_t::instance().register_error(
            error_code_t(core::error_level_t::fatal, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{5}),
            "TEST_FATAL_1", "fatal test");

        core::error_context_t ctx_debug(
            core::located_code_t{error_code_t(core::error_level_t::debug, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{1})},
            "debug message");
        plugin_registry_t::instance().notify_error(ctx_debug);
        EXPECT_EQ(plugin.call_count.load(), 0);

        core::error_context_t ctx_info(
            core::located_code_t{error_code_t(core::error_level_t::info, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{2})},
            "info message");
        plugin_registry_t::instance().notify_error(ctx_info);
        EXPECT_EQ(plugin.call_count.load(), 0);

        core::error_context_t ctx_warn(
            core::located_code_t{error_code_t(core::error_level_t::warn, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{3})},
            "warn message");
        plugin_registry_t::instance().notify_error(ctx_warn);
        EXPECT_EQ(plugin.call_count.load(), 0);

        core::error_context_t ctx_error(
            core::located_code_t{error_code_t(core::error_level_t::error, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{4})},
            "error message");
        plugin_registry_t::instance().notify_error(ctx_error);
        EXPECT_EQ(plugin.call_count.load(), 1);

        core::error_context_t ctx_fatal(
            core::located_code_t{error_code_t(core::error_level_t::fatal, domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{5})},
            "fatal message");
        plugin_registry_t::instance().notify_error(ctx_fatal);
        EXPECT_EQ(plugin.call_count.load(), 2);
    }

    TEST_F(plugin_registry_test_t, set_max_queue_size) {
        EXPECT_EQ(plugin_registry_t::instance().get_max_queue_size(), 0UL);
        plugin_registry_t::instance().set_max_queue_size(100);
        EXPECT_EQ(plugin_registry_t::instance().get_max_queue_size(), 100UL);
        plugin_registry_t::instance().set_max_queue_size(0);
        EXPECT_EQ(plugin_registry_t::instance().get_max_queue_size(), 0UL);
    }


    class deferred_notify_test_t : public ::testing::Test {
        protected:
        error_system::config::feature_flags_t::notify_mode_t original_mode_{
            error_system::config::feature_flags_t::notify_mode_t::sync};

        void SetUp() override {
            plugin_registry_t::instance().clear();
            plugin_registry_t::instance().clear_deferred_notifications();
            plugin_registry_t::instance().set_deferred_buffer_size(1024);

            original_mode_ = error_system::config::feature_flags_t::get_notify_mode();
            error_system::config::feature_flags_t::set_notify_mode(
                error_system::config::feature_flags_t::notify_mode_t::sync_deferred);

            auto& registry = error_system::core::error_registry_t::instance();
            registry.unregister_all();
            registry.set_duplicate_policy(error_system::core::duplicate_policy_t::skip);
            registry.register_error(
                error_system::core::error_code_t(0x800000000000002AULL), "TEST_CODE_42", "test");
            registry.register_error(
                error_code_t(error_system::core::error_level_t::error, error_system::domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{1}),
                "TEST_ERR_1", "test error 1");
        }

        void TearDown() override {
            error_system::config::feature_flags_t::set_notify_mode(original_mode_);
            plugin_registry_t::instance().clear_deferred_notifications();
            plugin_registry_t::instance().clear();
            error_system::core::error_registry_t::instance().unregister_all();
        }

        /**
         * @brief 构造错误上下文并触发通知
         * @details 新架构下 error_context_t 构造函数不通知，需显式调用
         *          plugin_registry_t::notify(ctx) 触发，由 registry 按
         *          sync_deferred 模式累积到线程本地缓冲。
         */
        error_system::core::error_context_t make_ctx(const std::string& message) {
            error_system::core::error_context_t ctx(
                error_system::core::located_code_t{error_system::core::error_code_t(0x800000000000002AULL)},
                message);
            plugin_registry_t::instance().notify(ctx);
            return ctx;
        }
    };

    TEST_F(deferred_notify_test_t, enqueue_buffers_without_notifying) {
        mock_plugin_t plugin("deferred_plugin");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        auto ctx = make_ctx("buffered error");

        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);
        EXPECT_EQ(plugin.call_count.load(), 0);
    }

    TEST_F(deferred_notify_test_t, flush_triggers_notifications) {
        mock_plugin_t plugin("flush_plugin");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        auto ctx1 = make_ctx("error 1");
        auto ctx2 = make_ctx("error 2");
        ASSERT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 2UL);

        plugin_registry_t::instance().flush_deferred_notifications();

        EXPECT_EQ(plugin.call_count.load(), 2);
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0UL);
    }

    TEST_F(deferred_notify_test_t, flush_empty_buffer_is_noop) {
        mock_plugin_t plugin("noop_plugin");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_EQ(plugin.call_count.load(), 0);
    }

    TEST_F(deferred_notify_test_t, clear_drops_buffered_notifications) {
        mock_plugin_t plugin("clear_plugin");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        auto ctx = make_ctx("to be dropped");
        ASSERT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);

        const size_t dropped = plugin_registry_t::instance().clear_deferred_notifications();
        EXPECT_EQ(dropped, 1UL);
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0UL);

        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_EQ(plugin.call_count.load(), 0);
    }

    TEST_F(deferred_notify_test_t, buffer_overflow_drops_new_notifications) {
        mock_plugin_t plugin("overflow_plugin");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        plugin_registry_t::instance().set_deferred_buffer_size(3);
        EXPECT_FALSE(plugin_registry_t::instance().deferred_buffer_overflowed());

        auto ctx = make_ctx("overflow test");
        ASSERT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);

        plugin_registry_t::instance().enqueue_deferred_notification(ctx);
        plugin_registry_t::instance().enqueue_deferred_notification(ctx);
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 3UL);
        EXPECT_FALSE(plugin_registry_t::instance().deferred_buffer_overflowed());

        plugin_registry_t::instance().enqueue_deferred_notification(ctx);
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 3UL);
        EXPECT_TRUE(plugin_registry_t::instance().deferred_buffer_overflowed());

        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_FALSE(plugin_registry_t::instance().deferred_buffer_overflowed());
        EXPECT_EQ(plugin.call_count.load(), 3);
    }

    TEST_F(deferred_notify_test_t, min_level_filtering_applies_to_deferred) {
        class level_filter_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "deferred_level_filter"; }
            core::error_level_t min_level() const noexcept override {
                return error_system::core::error_level_t::error;
            }
            void on_error(const core::error_context_t&) noexcept override {
                call_count.fetch_add(1);
            }
            std::atomic<int> call_count{0};
        };

        error_system::core::error_registry_t::instance().register_error(
            error_code_t(error_system::core::error_level_t::info, error_system::domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{9}),
            "TEST_INFO_9", "info test");

        level_filter_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);

        error_system::core::error_context_t info_ctx(
            error_system::core::located_code_t{error_code_t(error_system::core::error_level_t::info, error_system::domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{9})},
            "info level");
        plugin_registry_t::instance().notify(info_ctx);
        error_system::core::error_context_t error_ctx(
            error_system::core::located_code_t{error_code_t(error_system::core::error_level_t::error, error_system::domain::system_domain_t::application, core::subsystem_id_t{1}, core::module_id_t{1}, core::error_number_t{1})},
            "error level");
        plugin_registry_t::instance().notify(error_ctx);

        plugin_registry_t::instance().flush_deferred_notifications();

        EXPECT_EQ(plugin.call_count.load(), 1);
    }

    TEST_F(deferred_notify_test_t, set_deferred_buffer_size_zero_unlimited) {
        plugin_registry_t::instance().set_deferred_buffer_size(0);
        EXPECT_EQ(plugin_registry_t::instance().get_deferred_buffer_size(), 0UL);

        mock_plugin_t plugin("unlimited_plugin");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        error_system::core::error_context_t ctx(
            error_system::core::located_code_t{error_system::core::error_code_t(0x800000000000002AULL)}, "unlimited");
        for (int i = 0; i < 1099; ++i) {
            plugin_registry_t::instance().enqueue_deferred_notification(ctx);
        }
        EXPECT_FALSE(plugin_registry_t::instance().deferred_buffer_overflowed());
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1099UL);

        plugin_registry_t::instance().clear_deferred_notifications();
    }

    TEST_F(deferred_notify_test_t, deferred_buffer_is_thread_local) {
        mock_plugin_t plugin("tls_plugin");
        plugin_registry_t::instance().register_plugin_ref(plugin);

        auto ctx = make_ctx("main thread");
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);

        std::thread t([] {
            error_system::core::error_context_t child_ctx(
                error_system::core::located_code_t{error_system::core::error_code_t(0x800000000000002AULL)}, "child thread");
            plugin_registry_t::instance().notify(child_ctx);
            EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);
            plugin_registry_t::instance().flush_deferred_notifications();
            EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0UL);
        });
        t.join();

        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);
        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_EQ(plugin.call_count.load(), 2);
    }

    /**
     * @brief flush 期间插件回调构造新的 error_context_t 不应入队（flushing 标志防重入）
     */
    TEST_F(deferred_notify_test_t, flush_is_reentrant_safe) {
        class reentrant_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "reentrant"; }
            void on_error(const core::error_context_t&) noexcept override {
                call_count.fetch_add(1);
                core::error_context_t nested(core::located_code_t{core::error_code_t(0x800000000000002AULL)}, "nested during flush");
                plugin_registry_t::instance().notify(nested);
            }
            std::atomic<int> call_count{0};
        };

        reentrant_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);

        auto ctx = make_ctx("outer error");
        ASSERT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);

        plugin_registry_t::instance().flush_deferred_notifications();

        EXPECT_EQ(plugin.call_count.load(), 1);
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0UL);
    }

    /**
     * @brief flush 应保留缓冲上下文的字段（code、message）原样传递给插件
     */
    TEST_F(deferred_notify_test_t, flush_preserves_context_fields) {
        class capturing_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "capturer"; }
            void on_error(const core::error_context_t& ctx) noexcept override {
                call_count.fetch_add(1);
                seen_codes.push_back(ctx.get_code().get_code());
                seen_messages.emplace_back(ctx.get_message());
            }
            std::atomic<int> call_count{0};
            std::vector<uint64_t> seen_codes;
            std::vector<std::string> seen_messages;
        };

        capturing_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);

        auto ctx1 = make_ctx("first message");
        auto ctx2 = make_ctx("second message");

        plugin_registry_t::instance().flush_deferred_notifications();

        ASSERT_EQ(plugin.seen_codes.size(), 2UL);
        EXPECT_EQ(plugin.seen_codes[0], 0x800000000000002AULL);
        EXPECT_EQ(plugin.seen_codes[1], 0x800000000000002AULL);
        EXPECT_EQ(plugin.seen_messages[0], "first message");
        EXPECT_EQ(plugin.seen_messages[1], "second message");
    }

    /**
     * @brief 无插件注册时 flush 非空缓冲应安全完成且清空缓冲
     */
    TEST_F(deferred_notify_test_t, flush_with_no_plugins_is_noop) {
        auto ctx1 = make_ctx("no plugin 1");
        auto ctx2 = make_ctx("no plugin 2");
        ASSERT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 2UL);
        ASSERT_EQ(plugin_registry_t::instance().size(), 0UL);

        plugin_registry_t::instance().flush_deferred_notifications();

        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0UL);
    }

    /**
     * @brief 多插件 flush 时按快照顺序依次通知
     */
    TEST_F(deferred_notify_test_t, flush_dispatches_to_multiple_plugins_in_order) {
        class ordering_plugin_t : public i_error_plugin_t {
            public:
            explicit ordering_plugin_t(std::string n, std::vector<std::string>* log)
                : plugin_name(std::move(n)), call_log(log) {}
            std::string_view name() const noexcept override { return plugin_name; }
            void on_error(const core::error_context_t&) noexcept override {
                call_count.fetch_add(1);
                call_log->push_back(plugin_name);
            }
            std::string plugin_name;
            std::vector<std::string>* call_log;
            std::atomic<int> call_count{0};
        };

        std::vector<std::string> call_log;
        ordering_plugin_t first("first", &call_log);
        ordering_plugin_t second("second", &call_log);
        ordering_plugin_t third("third", &call_log);

        plugin_registry_t::instance().register_plugin_ref(first);
        plugin_registry_t::instance().register_plugin_ref(second);
        plugin_registry_t::instance().register_plugin_ref(third);

        auto ctx = make_ctx("ordering test");

        plugin_registry_t::instance().flush_deferred_notifications();

        ASSERT_EQ(call_log.size(), 3UL);
        EXPECT_EQ(call_log[0], "first");
        EXPECT_EQ(call_log[1], "second");
        EXPECT_EQ(call_log[2], "third");
    }

    TEST_F(plugin_registry_test_t, dispatch_to_plugins_filters_each_plugin_independently) {
        class tiered_plugin_t : public i_error_plugin_t {
            public:
            explicit tiered_plugin_t(std::string n, core::error_level_t threshold)
                : plugin_name(std::move(n)), min_threshold(threshold) {}
            std::string_view name() const noexcept override { return plugin_name; }
            core::error_level_t min_level() const noexcept override { return min_threshold; }
            void on_error(const core::error_context_t&) noexcept override {
                call_count.fetch_add(1);
            }
            std::string plugin_name;
            core::error_level_t min_threshold;
            std::atomic<int> call_count{0};
        };

        tiered_plugin_t info_plugin("info_tier", core::error_level_t::info);
        tiered_plugin_t error_plugin("error_tier", core::error_level_t::error);
        plugin_registry_t::instance().register_plugin_ref(info_plugin);
        plugin_registry_t::instance().register_plugin_ref(error_plugin);

        const auto info_code = error_code_t(core::error_level_t::info,
                                            domain::system_domain_t::application,
                                            core::subsystem_id_t{1}, core::module_id_t{1},
                                            core::error_number_t{71});
        const auto error_code = error_code_t(core::error_level_t::error,
                                             domain::system_domain_t::application,
                                             core::subsystem_id_t{1}, core::module_id_t{1},
                                             core::error_number_t{72});
        error_system::core::error_registry_t::instance().register_error(info_code, "TIER_INFO", "info");
        error_system::core::error_registry_t::instance().register_error(error_code, "TIER_ERROR", "error");

        core::error_context_t info_ctx(core::located_code_t{info_code}, "info level");
        plugin_registry_t::instance().notify_error(info_ctx);
        EXPECT_EQ(info_plugin.call_count.load(), 1);
        EXPECT_EQ(error_plugin.call_count.load(), 0);

        core::error_context_t error_ctx(core::located_code_t{error_code}, "error level");
        plugin_registry_t::instance().notify_error(error_ctx);
        EXPECT_EQ(info_plugin.call_count.load(), 2);
        EXPECT_EQ(error_plugin.call_count.load(), 1);
    }

    /**
     * @brief on_code 接口测试夹具
     * @details 专用于验证 Lean 模式纯错误码通知路径（notify(code) → on_code）。
     *          覆盖 sync/async/deferred 三种模式及 min_level 过滤。
     */
    class code_notify_test_t : public ::testing::Test {
        protected:
        error_system::config::feature_flags_t::notify_mode_t original_mode_{
            error_system::config::feature_flags_t::notify_mode_t::sync};

        void SetUp() override {
            plugin_registry_t::instance().clear();
            plugin_registry_t::instance().clear_deferred_notifications();
            original_mode_ = error_system::config::feature_flags_t::get_notify_mode();
        }

        void TearDown() override {
            error_system::config::feature_flags_t::set_notify_mode(original_mode_);
            plugin_registry_t::instance().clear_deferred_notifications();
            plugin_registry_t::instance().clear();
        }
    };

    TEST_F(code_notify_test_t, sync_notify_code_calls_on_code) {
        class code_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "code_plugin"; }
            void on_error(const core::error_context_t&) noexcept override {}
            void on_code(core::error_code_t c) noexcept override {
                last_code.store(c.get_code(), std::memory_order_relaxed);
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
            std::atomic<int> call_count{0};
            std::atomic<uint64_t> last_code{0};
        };

        code_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);
        config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync);

        const auto code = error_code_t(core::error_level_t::error,
                                       domain::system_domain_t::application,
                                       core::subsystem_id_t{1}, core::module_id_t{1},
                                       core::error_number_t{100});
        plugin_registry_t::instance().notify(code);

        EXPECT_EQ(plugin.call_count.load(), 1);
        EXPECT_EQ(plugin.last_code.load(std::memory_order_relaxed), code.get_code());
    }

    TEST_F(code_notify_test_t, default_on_code_is_noop) {
        class error_only_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "error_only"; }
            void on_error(const core::error_context_t&) noexcept override {
                call_count.fetch_add(1);
            }
            std::atomic<int> call_count{0};
        };

        error_only_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);
        config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync);

        const auto code = error_code_t(core::error_level_t::error,
                                       domain::system_domain_t::application,
                                       core::subsystem_id_t{1}, core::module_id_t{1},
                                       core::error_number_t{101});
        plugin_registry_t::instance().notify(code);

        EXPECT_EQ(plugin.call_count.load(), 0);
    }

    TEST_F(code_notify_test_t, notify_code_respects_min_level) {
        class level_code_plugin_t : public i_error_plugin_t {
            public:
            level_code_plugin_t(std::string n, core::error_level_t threshold)
                : plugin_name(std::move(n)), min_threshold(threshold) {}
            std::string_view name() const noexcept override { return plugin_name; }
            core::error_level_t min_level() const noexcept override { return min_threshold; }
            void on_error(const core::error_context_t&) noexcept override {}
            void on_code(core::error_code_t) noexcept override {
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
            std::string plugin_name;
            core::error_level_t min_threshold;
            std::atomic<int> call_count{0};
        };

        level_code_plugin_t error_plugin("error_tier", core::error_level_t::error);
        level_code_plugin_t fatal_plugin("fatal_tier", core::error_level_t::fatal);
        plugin_registry_t::instance().register_plugin_ref(error_plugin);
        plugin_registry_t::instance().register_plugin_ref(fatal_plugin);
        config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync);

        const auto warn_code = error_code_t(core::error_level_t::warn,
                                            domain::system_domain_t::application,
                                            core::subsystem_id_t{1}, core::module_id_t{1},
                                            core::error_number_t{102});
        plugin_registry_t::instance().notify(warn_code);
        EXPECT_EQ(error_plugin.call_count.load(), 0);
        EXPECT_EQ(fatal_plugin.call_count.load(), 0);

        const auto error_code_val = error_code_t(core::error_level_t::error,
                                                 domain::system_domain_t::application,
                                                 core::subsystem_id_t{1}, core::module_id_t{1},
                                                 core::error_number_t{103});
        plugin_registry_t::instance().notify(error_code_val);
        EXPECT_EQ(error_plugin.call_count.load(), 1);
        EXPECT_EQ(fatal_plugin.call_count.load(), 0);

        const auto fatal_code = error_code_t(core::error_level_t::fatal,
                                             domain::system_domain_t::application,
                                             core::subsystem_id_t{1}, core::module_id_t{1},
                                             core::error_number_t{104});
        plugin_registry_t::instance().notify(fatal_code);
        EXPECT_EQ(error_plugin.call_count.load(), 2);
        EXPECT_EQ(fatal_plugin.call_count.load(), 1);
    }

    TEST_F(code_notify_test_t, deferred_notify_code_buffers_and_flushes) {
        class code_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "deferred_code"; }
            void on_error(const core::error_context_t&) noexcept override {}
            void on_code(core::error_code_t) noexcept override {
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
            std::atomic<int> call_count{0};
        };

        code_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);
        plugin_registry_t::instance().set_deferred_buffer_size(1024);
        config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync_deferred);

        const auto code = error_code_t(core::error_level_t::error,
                                       domain::system_domain_t::application,
                                       core::subsystem_id_t{1}, core::module_id_t{1},
                                       core::error_number_t{105});
        plugin_registry_t::instance().notify(code);
        EXPECT_EQ(plugin.call_count.load(), 0);
        EXPECT_GE(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);

        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_EQ(plugin.call_count.load(), 1);
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0UL);
    }

    TEST_F(code_notify_test_t, clear_deferred_drops_code_buffer) {
        class code_plugin_t : public i_error_plugin_t {
            public:
            std::string_view name() const noexcept override { return "clear_code"; }
            void on_error(const core::error_context_t&) noexcept override {}
            void on_code(core::error_code_t) noexcept override {
                call_count.fetch_add(1, std::memory_order_relaxed);
            }
            std::atomic<int> call_count{0};
        };

        code_plugin_t plugin;
        plugin_registry_t::instance().register_plugin_ref(plugin);
        plugin_registry_t::instance().set_deferred_buffer_size(1024);
        config::feature_flags_t::set_notify_mode(config::feature_flags_t::notify_mode_t::sync_deferred);

        const auto code = error_code_t(core::error_level_t::error,
                                       domain::system_domain_t::application,
                                       core::subsystem_id_t{1}, core::module_id_t{1},
                                       core::error_number_t{106});
        plugin_registry_t::instance().notify(code);
        EXPECT_GE(plugin_registry_t::instance().pending_deferred_notifications(), 1UL);

        plugin_registry_t::instance().clear_deferred_notifications();
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0UL);

        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_EQ(plugin.call_count.load(), 0);
    }

}  // namespace error_system::plugin
