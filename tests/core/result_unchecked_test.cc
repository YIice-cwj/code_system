#include <cassert>
#include <string>
#include <type_traits>

#include <gtest/gtest.h>

#include "error_system/core/error_context.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/core/result.h"
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
 * @file result_unchecked_test.cc
 * @brief result_t 强制错误检查机制测试
 * @details 验证 Debug 构建下未检查错误即析构会触发 assert；
 *          Release 构建下整个测试套件仍可运行（EXPECT_DEATH 在 NDEBUG 下退化为无操作）。
 */

// ========== 正常路径：检查后析构不应触发断言 ==========

TEST(ResultUncheckedTest, success_result_no_check_safe_to_destroy) {
    result_t<int> r = result_t<int>::make_success(42);
    // 成功状态，未调用 is_error 也应安全析构
}

TEST(ResultUncheckedTest, error_result_checked_via_is_error_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    (void)r.is_error();
}

TEST(ResultUncheckedTest, error_result_checked_via_is_success_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    (void)r.is_success();
}

TEST(ResultUncheckedTest, error_result_checked_via_error_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    (void)r.error();
}

TEST(ResultUncheckedTest, success_result_checked_via_value_safe) {
    result_t<int> r = result_t<int>::make_success(42);
    (void)r.value();
}

TEST(ResultUncheckedTest, error_result_checked_via_operator_bool_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    if (r) { (void)0; }
}

TEST(ResultUncheckedTest, error_result_checked_via_value_pointer_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    (void)r.value_pointer();
}

TEST(ResultUncheckedTest, error_result_checked_via_value_or_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    (void)r.value_or(0);
}

TEST(ResultUncheckedTest, error_result_checked_via_match_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    int result = r.match([](const int&) { return 0; }, [](const error_context_t&) { return 1; });
    (void)result;
}

TEST(ResultUncheckedTest, void_error_result_checked_safe) {
    result_t<void> r = result_t<void>::make_error(make_test_error_code(), "err");
    (void)r.is_error();
}

TEST(ResultUncheckedTest, void_success_result_no_check_safe) {
    result_t<void> r = result_t<void>::make_success();
    // 成功状态，未调用 is_error 也应安全析构
}

// ========== 移动语义：源对象标记为已检查 ==========

TEST(ResultUncheckedTest, move_constructed_source_marked_checked) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    result_t<int> moved = std::move(r);
    (void)moved.is_error();
    // r 析构时不应触发断言（已被移动构造标记为 checked_）
}

TEST(ResultUncheckedTest, move_assigned_source_marked_checked) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    result_t<int> target = result_t<int>::make_success(0);
    target = std::move(r);
    (void)target.is_error();
    // r 析构时仍持有 error_context_t，但应被标记为 checked
}

TEST(ResultUncheckedTest, move_assign_overwrites_unchecked_self) {
    // target 自身持有未检查错误，赋值新值前应先检查自身（避免漏检）
    // 这里 target 是成功状态，无未检查错误，赋值安全
    result_t<int> target = result_t<int>::make_success(1);
    result_t<int> src = result_t<int>::make_success(2);
    target = std::move(src);
    EXPECT_EQ(target.value(), 2);
}

// ========== 拷贝语义：新对象需独立检查 ==========

TEST(ResultUncheckedTest, copied_result_must_be_checked_independently) {
    result_t<int> original = result_t<int>::make_error(make_test_error_code(), "err");
    (void)original.is_error();
    result_t<int> copy = original;
    // copy 析构前必须独立检查
    (void)copy.is_error();
}

// ========== 链式调用：消费后源对象标记已检查 ==========

TEST(ResultUncheckedTest, map_on_error_consumes_source_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    auto mapped = r.map([](const int& v) { return v * 2; });
    // r 通过 const& 调用 map，内部 is_error() 标记 checked_
    (void)mapped.is_error();
}

TEST(ResultUncheckedTest, and_then_on_error_consumes_source_safe) {
    result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
    auto next = std::move(r).and_then([](int&&) { return result_t<int>::make_success(0); });
    (void)next.is_error();
    // r 已被移动，析构安全
}

// ========== Debug 断言死亡测试（仅 NDEBUG 未定义时生效）==========

#ifndef NDEBUG
TEST(ResultUncheckedDeathTest, unchecked_error_result_triggers_assert) {
    // 死亡测试：构造错误 result 不检查直接析构，应触发 assert
    // 注意：stderr 输出 "unchecked error result destroyed" 后 abort
    EXPECT_DEATH(
        {
            result_t<int> r = result_t<int>::make_error(make_test_error_code(), "err");
            // 不调用任何检查方法，直接析构
        },
        "unchecked error result destroyed");
}

TEST(ResultUncheckedDeathTest, void_unchecked_error_result_triggers_assert) {
    EXPECT_DEATH(
        {
            result_t<void> r = result_t<void>::make_error(make_test_error_code(), "err");
            // 不检查直接析构
        },
        "unchecked error result destroyed");
}

TEST(ResultUncheckedDeathTest, unchecked_error_after_move_assign_overwrite_triggers_assert) {
    // target 持有未检查错误，被赋值新错误时应触发对自身的检查
    EXPECT_DEATH(
        {
            result_t<int> target = result_t<int>::make_error(make_test_error_code(1), "first");
            result_t<int> src = result_t<int>::make_error(make_test_error_code(2), "second");
            target = std::move(src);
            // target 析构时 src 已被标记，但 src 自身的 error_context 已被移走，
            // 此处主要验证 target 析构安全（target 持有 src 的错误，未检查）
        },
        "unchecked error result destroyed");
}
#endif  // NDEBUG
