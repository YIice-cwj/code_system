#include <string>
#include <type_traits>

#include <gtest/gtest.h>

#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/core/result/result.h"
#include "error_system/domain/system_domain.h"

using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::result_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

namespace {
    /**
     * @brief 构造测试用错误码
     */
    error_code_t make_test_error_code(uint16_t number = 1) {
        return error_code_t{error_level_t::error,
                            system_domain_t::application,
                            subsystem_id_t{1},
                            module_id_t{1},
                            error_number_t{number}};
    }
}  // namespace

/**
 * @file result_lean_test.cc
 * @brief result_t<T, true> Lean 模式单元测试
 * @details 验证 Lean 模式下错误路径仅携带 error_code_t，成功路径与完整模式一致。
 */

TEST(ResultLeanTest, lean_error_result_stores_only_error_code) {
    auto r = result_t<int, true>::make_error(make_test_error_code(42), "ignored message");
    EXPECT_TRUE(r.is_error());
    EXPECT_EQ(r.error_code().get_number(), 42);
}

TEST(ResultLeanTest, lean_success_result_stores_value) {
    auto r = result_t<int, true>::make_success(100);
    EXPECT_TRUE(r.is_success());
    EXPECT_EQ(r.value(), 100);
}

TEST(ResultLeanTest, lean_error_returns_temporary_context_with_code) {
    auto r = result_t<int, true>::make_error(make_test_error_code(7), "msg");
    error_context_t ctx = r.error();
    EXPECT_EQ(ctx.get_code().get_number(), 7);
}

TEST(ResultLeanTest, lean_void_error_result_stores_error_code) {
    auto r = result_t<void, true>::make_error(make_test_error_code(99), "msg");
    EXPECT_TRUE(r.is_error());
    EXPECT_EQ(r.error_code().get_number(), 99);
}

TEST(ResultLeanTest, lean_void_success_result_safe) {
    auto r = result_t<void, true>::make_success();
    EXPECT_TRUE(r.is_success());
}

TEST(ResultLeanTest, lean_error_code_extracted_from_context) {
    error_context_t ctx = error_context_t::make_minimal(make_test_error_code(55));
    auto r = result_t<int, true>::make_error(std::move(ctx));
    EXPECT_TRUE(r.is_error());
    EXPECT_EQ(r.error_code().get_number(), 55);
}

TEST(ResultLeanTest, lean_map_propagates_error_code) {
    auto r = result_t<int, true>::make_error(make_test_error_code(33), "err");
    auto mapped = r.map([](const int& v) { return v * 2; });
    EXPECT_TRUE(mapped.is_error());
    EXPECT_EQ(mapped.error_code().get_number(), 33);
}

TEST(ResultLeanTest, lean_map_transforms_success_value) {
    auto r = result_t<int, true>::make_success(21);
    auto mapped = r.map([](const int& v) { return v * 2; });
    EXPECT_TRUE(mapped.is_success());
    EXPECT_EQ(mapped.value(), 42);
}

TEST(ResultLeanTest, lean_and_then_propagates_error_code) {
    auto r = result_t<int, true>::make_error(make_test_error_code(11), "err");
    auto next = std::move(r).and_then([](int&&) { return result_t<int, true>::make_success(0); });
    EXPECT_TRUE(next.is_error());
    EXPECT_EQ(next.error_code().get_number(), 11);
}

TEST(ResultLeanTest, lean_and_then_chains_success) {
    auto r = result_t<int, true>::make_success(10);
    auto next = std::move(r).and_then([](int&& v) { return result_t<int, true>::make_success(v + 5); });
    EXPECT_TRUE(next.is_success());
    EXPECT_EQ(next.value(), 15);
}

TEST(ResultLeanTest, lean_match_dispatches_to_error_branch) {
    auto r = result_t<int, true>::make_error(make_test_error_code(8), "err");
    int result = r.match([](const int&) { return 0; }, [](const error_context_t& ctx) {
        return static_cast<int>(ctx.get_code().get_number());
    });
    EXPECT_EQ(result, 8);
}

TEST(ResultLeanTest, lean_match_dispatches_to_success_branch) {
    auto r = result_t<int, true>::make_success(99);
    int result = r.match([](const int& v) { return v; }, [](const error_context_t&) { return -1; });
    EXPECT_EQ(result, 99);
}

TEST(ResultLeanTest, lean_move_constructed_source_safe) {
    auto r = result_t<int, true>::make_error(make_test_error_code(1), "err");
    result_t<int, true> moved = std::move(r);
    EXPECT_TRUE(moved.is_error());
}

TEST(ResultLeanTest, lean_move_assigned_source_safe) {
    auto r = result_t<int, true>::make_error(make_test_error_code(2), "err");
    auto target = result_t<int, true>::make_success(0);
    target = std::move(r);
    EXPECT_TRUE(target.is_error());
    EXPECT_EQ(target.error_code().get_number(), 2);
}

TEST(ResultLeanTest, lean_error_code_on_success_returns_success_code) {
    auto r = result_t<int, true>::make_success(42);
    EXPECT_TRUE(r.error_code().is_success_code());
}

TEST(ResultLeanTest, lean_size_smaller_than_full_mode) {
    static_assert(sizeof(result_t<int, true>) <= sizeof(result_t<int, false>),
                  "lean result_t should not be larger than full result_t");
}
