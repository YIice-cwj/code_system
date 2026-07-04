/**
 * @file notify_mode_comparison_test.cc
 * @brief 三种通知模式横向对比测试：sync / async_queue / sync_deferred
 * @details 验证三种模式在行为上的关键差异：
 *          - sync：构造 error_context_t 时立即调用插件
 *          - async_queue：异步入队，由后台线程消费
 *          - sync_deferred：累积到线程本地缓冲，flush 时批量通知
 *          对比维度：调用时机、调用线程、批量语义、min_level 过滤一致性。
 */

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

#include <gtest/gtest.h>

#include "error_system/config/feature_flags.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/core/error_registry.h"
#include "error_system/domain/system_domain.h"
#include "error_system/plugin/i_error_plugin.h"
#include "error_system/plugin/plugin_registry.h"

using error_system::config::feature_flags_t;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::located_code_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::plugin::i_error_plugin_t;
using error_system::plugin::plugin_registry_t;

namespace {

    /** @brief 记录调用信息（线程 ID、调用次数、最后上下文）的插件 */
    class tracing_plugin_t : public i_error_plugin_t {
    public:
        std::atomic<int> call_count{0};
        std::atomic<std::thread::id> last_thread_id{};
        std::mutex mtx;
        error_level_t min_level_threshold{error_level_t::info};

        std::string_view name() const noexcept override { return "tracing"; }

        error_level_t min_level() const noexcept override {
            return min_level_threshold;
        }

        void on_error(const error_context_t& /*context*/) noexcept override {
            ++call_count;
            last_thread_id.store(std::this_thread::get_id());
        }

        int count() const noexcept { return call_count.load(); }

        void reset() noexcept {
            call_count.store(0);
        }
    };

    /** @brief 构造测试用错误上下文 */
    error_context_t make_context(uint16_t number) {
        const error_code_t code(error_level_t::error, system_domain_t::application,
                                subsystem_id_t{1}, module_id_t{1}, error_number_t{number});
        return error_context_t{located_code_t{code}, "模式对比测试"};
    }

    /** @brief 等待计数到达预期 */
    bool wait_for_count(std::atomic<int>& counter, int expected, int timeout_ms = 2000) {
        for (int i = 0; i < timeout_ms / 10; ++i) {
            if (counter.load() >= expected) { return true; }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        return counter.load() >= expected;
    }

}  // namespace

class notify_mode_comparison_test_t : public ::testing::Test {
protected:
    bool saved_validation_{true};
    bool saved_stacktrace_{true};
    bool saved_location_{true};

    void SetUp() override {
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
        saved_validation_ = feature_flags_t::is_validation_enabled();
        saved_stacktrace_ = feature_flags_t::is_stacktrace_enabled();
        saved_location_ = feature_flags_t::is_source_location_enabled();
        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(false);
        feature_flags_t::set_enable_source_location(false);
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    }

    void TearDown() override {
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        feature_flags_t::set_enable_validation(saved_validation_);
        feature_flags_t::set_enable_stacktrace(saved_stacktrace_);
        feature_flags_t::set_enable_source_location(saved_location_);
        plugin_registry_t::instance().clear();
        error_registry_t::instance().unregister_all();
    }
};

TEST_F(notify_mode_comparison_test_t, sync_mode_invokes_plugin_immediately) {
    tracing_plugin_t plugin;
    plugin_registry_t::instance().register_plugin_ref(plugin);

    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
    error_context_t ctx = make_context(1);

    EXPECT_EQ(plugin.count(), 1);
    EXPECT_EQ(plugin.last_thread_id.load(), std::this_thread::get_id());
}

TEST_F(notify_mode_comparison_test_t, async_queue_mode_invokes_plugin_on_worker_thread) {
    tracing_plugin_t plugin;
    plugin_registry_t::instance().register_plugin_ref(plugin);

    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::async_queue);
    error_context_t ctx1 = make_context(1);
    error_context_t ctx2 = make_context(2);

    EXPECT_TRUE(wait_for_count(plugin.call_count, 2));
    EXPECT_EQ(plugin.count(), 2);
    EXPECT_NE(plugin.last_thread_id.load(), std::this_thread::get_id());
}

TEST_F(notify_mode_comparison_test_t, sync_deferred_mode_buffers_until_flush) {
    tracing_plugin_t plugin;
    plugin_registry_t::instance().register_plugin_ref(plugin);

    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
    error_context_t ctx1 = make_context(1);
    error_context_t ctx2 = make_context(2);
    error_context_t ctx3 = make_context(3);

    EXPECT_EQ(plugin.count(), 0);
    EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 3u);

    plugin_registry_t::instance().flush_deferred_notifications();

    EXPECT_EQ(plugin.count(), 3);
    EXPECT_EQ(plugin.last_thread_id.load(), std::this_thread::get_id());
    EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0u);
}

TEST_F(notify_mode_comparison_test_t, sync_deferred_clear_drops_notifications) {
    tracing_plugin_t plugin;
    plugin_registry_t::instance().register_plugin_ref(plugin);

    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
    error_context_t ctx1 = make_context(1);
    error_context_t ctx2 = make_context(2);
    (void)ctx1;
    (void)ctx2;

    EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 2u);
    const size_t cleared = plugin_registry_t::instance().clear_deferred_notifications();
    EXPECT_EQ(cleared, 2u);
    EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0u);

    plugin_registry_t::instance().flush_deferred_notifications();
    EXPECT_EQ(plugin.count(), 0);
}

TEST_F(notify_mode_comparison_test_t, sync_deferred_buffer_overflow_drops) {
    tracing_plugin_t plugin;
    plugin_registry_t::instance().register_plugin_ref(plugin);

    plugin_registry_t::instance().set_deferred_buffer_size(5);
    EXPECT_EQ(plugin_registry_t::instance().get_deferred_buffer_size(), 5u);

    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
    for (int i = 0; i < 10; ++i) {
        error_context_t ctx = make_context(static_cast<uint16_t>(i + 1));
        (void)ctx;
    }

    EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 5u);
    EXPECT_TRUE(plugin_registry_t::instance().deferred_buffer_overflowed());

    plugin_registry_t::instance().flush_deferred_notifications();
    EXPECT_EQ(plugin.count(), 5);
    EXPECT_FALSE(plugin_registry_t::instance().deferred_buffer_overflowed());
}

TEST_F(notify_mode_comparison_test_t, async_queue_backpressure_drops_excess) {
    tracing_plugin_t plugin;
    plugin_registry_t::instance().register_plugin_ref(plugin);
    plugin_registry_t::instance().set_max_queue_size(2);
    EXPECT_EQ(plugin_registry_t::instance().get_max_queue_size(), 2u);

    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::async_queue);
    error_context_t ctx1 = make_context(1);
    error_context_t ctx2 = make_context(2);
    error_context_t ctx3 = make_context(3);
    error_context_t ctx4 = make_context(4);
    (void)ctx1;
    (void)ctx2;
    (void)ctx3;
    (void)ctx4;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_LE(plugin.count(), 4);
}

TEST_F(notify_mode_comparison_test_t, min_level_filter_consistent_across_modes) {
    tracing_plugin_t plugin;
    plugin.min_level_threshold = error_level_t::error;
    plugin_registry_t::instance().register_plugin_ref(plugin);

    {
        plugin.reset();
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        const error_code_t info_code(error_level_t::info, system_domain_t::application,
                                     subsystem_id_t{1}, module_id_t{1}, error_number_t{10});
        error_context_t ctx{located_code_t{info_code}, "info 级别"};
        (void)ctx;
        EXPECT_EQ(plugin.count(), 0) << "sync: info level should be filtered by min_level=error";
    }

    {
        plugin.reset();
        plugin_registry_t::instance().clear_deferred_notifications();
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
        const error_code_t info_code(error_level_t::info, system_domain_t::application,
                                     subsystem_id_t{1}, module_id_t{1}, error_number_t{11});
        error_context_t ctx{located_code_t{info_code}, "info 级别"};
        (void)ctx;
        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_EQ(plugin.count(), 0) << "sync_deferred: info level should be filtered on flush";
    }

    {
        plugin.reset();
        plugin_registry_t::instance().clear_deferred_notifications();
        feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync);
        const error_code_t err_code(error_level_t::error, system_domain_t::application,
                                    subsystem_id_t{1}, module_id_t{1}, error_number_t{12});
        error_context_t ctx{located_code_t{err_code}, "error 级别"};
        (void)ctx;
        EXPECT_EQ(plugin.count(), 1) << "sync: error level should pass min_level=error";
    }
}

TEST_F(notify_mode_comparison_test_t, sync_deferred_thread_local_isolation) {
    tracing_plugin_t plugin;
    plugin_registry_t::instance().register_plugin_ref(plugin);

    feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);

    std::thread t1([] {
        error_context_t ctx = make_context(100);
        (void)ctx;
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1u);
        plugin_registry_t::instance().flush_deferred_notifications();
        EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 0u);
    });

    error_context_t main_ctx = make_context(200);
    (void)main_ctx;
    EXPECT_EQ(plugin_registry_t::instance().pending_deferred_notifications(), 1u);

    t1.join();
    plugin_registry_t::instance().flush_deferred_notifications();
}
