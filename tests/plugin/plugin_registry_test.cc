#include "error_system/core/error_builder.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/plugin/plugin_registry.h"

#include <atomic>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace error_system::plugin {

    class plugin_registry_test : public ::testing::Test {
        protected:
        void SetUp() override {
            plugin_registry_t::instance().clear();
        }

        void TearDown() override {
            plugin_registry_t::instance().clear();
        }
    };

    /**
     * @brief 计数插件，用于验证通知是否被正确触发
     */
    class counting_plugin_t : public i_error_plugin_t {
        private:
        std::string name_;
        std::atomic<size_t> count_{0};

        public:
        explicit counting_plugin_t(std::string name) : name_(std::move(name)) {}

        std::string_view name() const noexcept override { return name_; }

        void on_error(const core::error_context_t&) noexcept override { ++count_; }

        size_t count() const noexcept { return count_.load(); }
    };

    TEST_F(plugin_registry_test, register_and_notify_single_thread) {
        counting_plugin_t plugin("test_plugin_0");
        auto& registry = plugin_registry_t::instance();

        registry.register_plugin(&plugin);
        EXPECT_EQ(registry.size(), 1);

        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::system, 0, 0, 1);
        core::error_context_t ctx(code, "test");

        EXPECT_EQ(plugin.count(), 1);
    }

    TEST_F(plugin_registry_test, unregister_plugin_removes_it) {
        counting_plugin_t plugin("test_plugin_0");
        auto& registry = plugin_registry_t::instance();

        registry.register_plugin(&plugin);
        EXPECT_EQ(registry.size(), 1);

        registry.unregister_plugin("test_plugin_0");
        EXPECT_EQ(registry.size(), 0);
        EXPECT_TRUE(registry.empty());
    }

    TEST_F(plugin_registry_test, concurrent_register_unregister) {
        auto& registry = plugin_registry_t::instance();
        constexpr int thread_count = 8;
        constexpr int iterations = 100;

        std::vector<std::unique_ptr<counting_plugin_t>> plugins;
        for (int i = 0; i < thread_count; ++i) {
            plugins.push_back(std::make_unique<counting_plugin_t>("concurrent_plugin_" + std::to_string(i)));
        }

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&, i]() {
                for (int j = 0; j < iterations; ++j) {
                    registry.register_plugin(plugins[i].get());
                    registry.unregister_plugin(plugins[i]->name());
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        EXPECT_EQ(registry.size(), 0);
        EXPECT_TRUE(registry.empty());
    }

    TEST_F(plugin_registry_test, concurrent_notify_with_plugins) {
        auto& registry = plugin_registry_t::instance();
        constexpr int plugin_count = 4;
        constexpr int thread_count = 8;
        constexpr int notify_per_thread = 100;

        std::vector<std::unique_ptr<counting_plugin_t>> plugins;
        plugins.push_back(std::make_unique<counting_plugin_t>("test_plugin_0"));
        plugins.push_back(std::make_unique<counting_plugin_t>("test_plugin_1"));
        plugins.push_back(std::make_unique<counting_plugin_t>("test_plugin_2"));
        plugins.push_back(std::make_unique<counting_plugin_t>("test_plugin_3"));

        for (int i = 0; i < plugin_count; ++i) {
            registry.register_plugin(plugins[i].get());
        }

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < notify_per_thread; ++j) {
                    auto code = core::error_builder_t::make_error_code(
                        core::error_level_t::error, domain::system_domain_t::system, 0, 0, 1);
                    core::error_context_t ctx(code, "concurrent notify");
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        size_t total = 0;
        for (int i = 0; i < plugin_count; ++i) {
            total += plugins[i]->count();
        }
        EXPECT_EQ(total, static_cast<size_t>(thread_count * notify_per_thread * plugin_count));
    }

    TEST_F(plugin_registry_test, concurrent_mixed_operations) {
        auto& registry = plugin_registry_t::instance();
        constexpr int thread_count = 8;
        constexpr int iterations = 50;

        counting_plugin_t plugin_a("concurrent_plugin_a");
        counting_plugin_t plugin_b("concurrent_plugin_b");

        registry.register_plugin(&plugin_a);
        registry.register_plugin(&plugin_b);

        std::vector<std::thread> threads;
        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back([&]() {
                for (int j = 0; j < iterations; ++j) {
                    auto code = core::error_builder_t::make_error_code(
                        core::error_level_t::error, domain::system_domain_t::system, 0, 0, 1);
                    core::error_context_t ctx(code, "mixed");

                    if (j % 5 == 0) {
                        registry.unregister_plugin("concurrent_plugin_a");
                        registry.register_plugin(&plugin_a);
                    }
                    if (j % 7 == 0) {
                        registry.unregister_plugin("concurrent_plugin_b");
                        registry.register_plugin(&plugin_b);
                    }
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }

        EXPECT_LE(registry.size(), 2);
    }

}  // namespace error_system::plugin
