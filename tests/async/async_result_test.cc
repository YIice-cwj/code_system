#include <chrono>
#include <stdexcept>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "error_system/async/async_result.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/core/result.h"
#include "error_system/domain/system_domain.h"

using error_system::async::async_result_t;
using error_system::async::make_async;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::result_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

namespace {
    error_code_t make_test_error_code(uint16_t number = 1) {
        return error_code_t{error_level_t::error,
                            system_domain_t::application,
                            subsystem_id_t{1},
                            module_id_t{1},
                            error_number_t{number}};
    }

    result_t<int> make_sync_success(int value) {
        return result_t<int>::make_success(value);
    }

    result_t<int> make_sync_error(uint16_t number) {
        error_context_t context = error_context_t::make_minimal(make_test_error_code(number));
        return result_t<int>::make_error(context);
    }
}

/**
 * @file async_result_test.cc
 * @brief async_result_t 单元测试
 * @details 验证异步结果链式 then/recover API、异常转换、Lean 模式支持。
 */

TEST(AsyncResultTest, construct_from_future_get_returns_result) {
    std::future<result_t<int>> future = std::async(std::launch::async, [] { return make_sync_success(42); });
    async_result_t<int> async_result(std::move(future));
    auto result = async_result.get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 42);
}

TEST(AsyncResultTest, construct_from_ready_result) {
    async_result_t<int> async_result(make_sync_success(99));
    auto result = async_result.get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 99);
}

TEST(AsyncResultTest, wait_and_is_ready) {
    async_result_t<int> async_result(std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return make_sync_success(1);
    }));
    EXPECT_FALSE(async_result.is_ready());
    async_result.wait();
    EXPECT_TRUE(async_result.is_ready());
}

TEST(AsyncResultTest, valid_after_construction) {
    async_result_t<int> async_result(make_sync_success(0));
    EXPECT_TRUE(async_result.valid());
    (void)async_result.get();
    EXPECT_FALSE(async_result.valid());
}

TEST(AsyncResultTest, then_transforms_success_value) {
    auto async_result = make_async([] { return make_sync_success(10); })
                            .then([](result_t<int> res) { return result_t<int>::make_success(res.value() * 2); });
    auto result = async_result.get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 20);
}

TEST(AsyncResultTest, then_chains_multiple_transforms) {
    auto result = make_async([] { return make_sync_success(1); })
                      .then([](result_t<int> res) { return result_t<int>::make_success(res.value() + 10); })
                      .then([](result_t<int> res) { return result_t<int>::make_success(res.value() * 3); })
                      .get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 33);
}

TEST(AsyncResultTest, then_propagates_error_unchanged) {
    auto result = make_async([] { return make_sync_error(7); })
                      .then([](result_t<int> res) {
                          if (res.is_error()) { return result_t<int>::make_error(res.error()); }
                          return result_t<int>::make_success(res.value());
                      })
                      .get();
    ASSERT_TRUE(result.is_error());
    EXPECT_EQ(result.error_code().get_number(), 7);
}

TEST(AsyncResultTest, then_changes_value_type) {
    auto result = make_async([] { return make_sync_success(5); })
                      .then([](result_t<int> res) { return result_t<std::string>::make_success(std::to_string(res.value())); })
                      .get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), "5");
}

TEST(AsyncResultTest, recover_recovers_from_error) {
    auto result = make_async([] { return make_sync_error(42); })
                      .recover([](error_context_t) { return result_t<int>::make_success(0); })
                      .get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 0);
}

TEST(AsyncResultTest, recover_passes_success_unchanged) {
    auto result = make_async([] { return make_sync_success(77); })
                      .recover([](error_context_t) { return result_t<int>::make_success(0); })
                      .get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 77);
}

TEST(AsyncResultTest, then_recover_chain) {
    auto result = make_async([] { return make_sync_error(1); })
                      .then([](result_t<int> res) { return res; })
                      .recover([](error_context_t) { return result_t<int>::make_success(999); })
                      .get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 999);
}

TEST(AsyncResultTest, then_catches_callback_exception) {
    auto result = make_async([] { return make_sync_success(1); })
                      .then([](result_t<int>) -> result_t<int> { throw std::runtime_error("boom"); })
                      .get();
    ASSERT_TRUE(result.is_error());
    EXPECT_EQ(result.error_code().get_level(), error_level_t::fatal);
}

TEST(AsyncResultTest, recover_catches_callback_exception) {
    auto result = make_async([] { return make_sync_error(1); })
                      .recover([](error_context_t) -> result_t<int> { throw std::runtime_error("recover failed"); })
                      .get();
    ASSERT_TRUE(result.is_error());
    EXPECT_EQ(result.error_code().get_level(), error_level_t::fatal);
}

TEST(AsyncResultTest, make_async_catches_function_exception) {
    auto result = make_async([]() -> result_t<int> { throw std::runtime_error("factory failed"); }).get();
    ASSERT_TRUE(result.is_error());
    EXPECT_EQ(result.error_code().get_level(), error_level_t::fatal);
}

TEST(AsyncResultTest, get_catches_inner_future_exception) {
    std::future<result_t<int>> future = std::async(std::launch::async, []() -> result_t<int> {
        throw std::runtime_error("inner");
    });
    async_result_t<int> async_result(std::move(future));
    auto result = async_result.get();
    ASSERT_TRUE(result.is_error());
    EXPECT_EQ(result.error_code().get_level(), error_level_t::fatal);
}

TEST(AsyncResultTest, void_result_supports_then) {
    auto result = make_async([] { return result_t<void>::make_success(); })
                      .then([](result_t<void> res) {
                          if (res.is_success()) { return result_t<int>::make_success(1); }
                          return result_t<int>::make_error(error_code_t::make_success());
                      })
                      .get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 1);
}

TEST(AsyncResultTest, lean_mode_then_propagates_error_code) {
    auto result = make_async([] { return result_t<int, true>::make_error(make_test_error_code(33), "err"); })
                      .then([](result_t<int, true> res) { return res; })
                      .get();
    ASSERT_TRUE(result.is_error());
    EXPECT_EQ(result.error_code().get_number(), 33);
}

TEST(AsyncResultTest, lean_mode_recover_restores_success) {
    auto result = make_async([] { return result_t<int, true>::make_error(make_test_error_code(1), "err"); })
                      .recover([](error_context_t) { return result_t<int, true>::make_success(42); })
                      .get();
    ASSERT_TRUE(result.is_success());
    EXPECT_EQ(result.value(), 42);
}
