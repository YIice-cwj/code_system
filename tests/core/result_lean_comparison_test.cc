/**
 * @file result_lean_comparison_test.cc
 * @brief result_t<T, true> (Lean) vs result_t<T, false> (完整) 行为对比测试
 * @details 横向对比两种模式在相同操作下的行为差异与一致性：
 *          - 成功路径完全一致（值访问、operator*、value_or、map、and_then）
 *          - 错误路径：Lean 仅携带 error_code_t，完整携带 error_context_t
 *          - error() 返回类型不同（值 vs 引用）
 *          - error_code() 行为一致
 *          - context() 仅完整模式可用
 *          - 内存占用：Lean 不大于完整
 *          - make_error 工厂语义一致
 */

#include <string>
#include <type_traits>

#include <gtest/gtest.h>

#include "error_system/config/feature_flags.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/core/result.h"
#include "error_system/domain/system_domain.h"

using error_system::config::feature_flags_t;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::result_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

namespace {

    error_code_t make_err(uint16_t number = 1) {
        return error_code_t{error_level_t::error, system_domain_t::application,
                            subsystem_id_t{1}, module_id_t{1}, error_number_t{number}};
    }

}  // namespace

class result_lean_comparison_test_t : public ::testing::Test {
protected:
    bool saved_validation_{true};
    bool saved_stacktrace_{true};
    bool saved_location_{true};

    void SetUp() override {
        saved_validation_ = feature_flags_t::is_validation_enabled();
        saved_stacktrace_ = feature_flags_t::is_stacktrace_enabled();
        saved_location_ = feature_flags_t::is_source_location_enabled();
        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(false);
        feature_flags_t::set_enable_source_location(false);
    }

    void TearDown() override {
        feature_flags_t::set_enable_validation(saved_validation_);
        feature_flags_t::set_enable_stacktrace(saved_stacktrace_);
        feature_flags_t::set_enable_source_location(saved_location_);
    }
};

TEST_F(result_lean_comparison_test_t, success_value_identical_between_modes) {
    auto full = result_t<int, false>::make_success(42);
    auto lean = result_t<int, true>::make_success(42);

    EXPECT_EQ(full.is_success(), lean.is_success());
    EXPECT_EQ(full.value(), lean.value());
    EXPECT_EQ(*full, *lean);
    EXPECT_EQ(full.value_or(0), lean.value_or(0));
}

TEST_F(result_lean_comparison_test_t, success_value_pointer_identical_between_modes) {
    auto full = result_t<int, false>::make_success(42);
    auto lean = result_t<int, true>::make_success(42);

    EXPECT_NE(full.value_pointer(), nullptr);
    EXPECT_NE(lean.value_pointer(), nullptr);
    EXPECT_EQ(*full.value_pointer(), *lean.value_pointer());
}

TEST_F(result_lean_comparison_test_t, error_state_identical_between_modes) {
    auto full = result_t<int, false>::make_error(make_err(7), "full err");
    auto lean = result_t<int, true>::make_error(make_err(7), "lean err");

    EXPECT_EQ(full.is_error(), lean.is_error());
    EXPECT_EQ(full.error_code().get_number(), lean.error_code().get_number());
    EXPECT_EQ(full.error_code().get_code(), lean.error_code().get_code());
}

TEST_F(result_lean_comparison_test_t, error_code_on_success_identical_between_modes) {
    auto full = result_t<int, false>::make_success(42);
    auto lean = result_t<int, true>::make_success(42);

    EXPECT_TRUE(full.error_code().is_success_code());
    EXPECT_TRUE(lean.error_code().is_success_code());
    EXPECT_EQ(full.error_code().get_code(), lean.error_code().get_code());
}

TEST_F(result_lean_comparison_test_t, error_returns_different_types) {
    const auto full = result_t<int, false>::make_error(make_err(7), "full");
    const auto lean = result_t<int, true>::make_error(make_err(7), "lean");

    static_assert(std::is_same_v<decltype(full.error()), const error_context_t&>,
                  "full mode error() must return const reference");
    static_assert(std::is_same_v<decltype(lean.error()), error_context_t>,
                  "lean mode error() must return value");

    (void)full.error();
    (void)lean.error();
}

TEST_F(result_lean_comparison_test_t, error_message_not_preserved_in_lean_mode) {
    auto full = result_t<int, false>::make_error(make_err(7), "完整模式消息");
    auto lean = result_t<int, true>::make_error(make_err(7), "Lean 模式消息");

    EXPECT_EQ(full.error().message, "完整模式消息");
    EXPECT_NE(lean.error().message, "Lean 模式消息");
}

TEST_F(result_lean_comparison_test_t, error_payload_not_preserved_in_lean_mode) {
    error_context_t ctx{error_system::core::located_code_t{make_err(7)}, "带 payload"};
    ctx.with("user_id", "8848");

    auto full = result_t<int, false>::make_error(ctx);
    auto lean = result_t<int, true>::make_error(ctx);

    EXPECT_EQ(full.error().payload_size(), 1u);
    EXPECT_EQ(lean.error().payload_size(), 0u);
}

TEST_F(result_lean_comparison_test_t, map_success_identical_between_modes) {
    auto full = result_t<int, false>::make_success(21);
    auto lean = result_t<int, true>::make_success(21);

    auto full_mapped = full.map([](const int& v) { return v * 2; });
    auto lean_mapped = lean.map([](const int& v) { return v * 2; });

    EXPECT_EQ(full_mapped.value(), lean_mapped.value());
    EXPECT_EQ(full_mapped.value(), 42);
}

TEST_F(result_lean_comparison_test_t, map_error_propagates_identical_between_modes) {
    auto full = result_t<int, false>::make_error(make_err(33), "err");
    auto lean = result_t<int, true>::make_error(make_err(33), "err");

    auto full_mapped = full.map([](const int& v) { return v * 2; });
    auto lean_mapped = lean.map([](const int& v) { return v * 2; });

    EXPECT_EQ(full_mapped.is_error(), lean_mapped.is_error());
    EXPECT_EQ(full_mapped.error_code().get_number(), lean_mapped.error_code().get_number());
}

TEST_F(result_lean_comparison_test_t, and_then_success_identical_between_modes) {
    auto full = result_t<int, false>::make_success(10);
    auto lean = result_t<int, true>::make_success(10);

    auto full_next = std::move(full).and_then([](int&& v) {
        return result_t<int, false>::make_success(v + 5);
    });
    auto lean_next = std::move(lean).and_then([](int&& v) {
        return result_t<int, true>::make_success(v + 5);
    });

    EXPECT_EQ(full_next.value(), lean_next.value());
    EXPECT_EQ(full_next.value(), 15);
}

TEST_F(result_lean_comparison_test_t, and_then_error_propagates_identical_between_modes) {
    auto full = result_t<int, false>::make_error(make_err(11), "err");
    auto lean = result_t<int, true>::make_error(make_err(11), "err");

    auto full_next = std::move(full).and_then([](int&&) {
        return result_t<int, false>::make_success(0);
    });
    auto lean_next = std::move(lean).and_then([](int&&) {
        return result_t<int, true>::make_success(0);
    });

    EXPECT_EQ(full_next.is_error(), lean_next.is_error());
    EXPECT_EQ(full_next.error_code().get_number(), lean_next.error_code().get_number());
}

TEST_F(result_lean_comparison_test_t, match_success_identical_between_modes) {
    auto full = result_t<int, false>::make_success(99);
    auto lean = result_t<int, true>::make_success(99);

    auto full_result = full.match([](const int& v) { return v; },
                                  [](const error_context_t&) { return -1; });
    auto lean_result = lean.match([](const int& v) { return v; },
                                  [](const error_context_t&) { return -1; });

    EXPECT_EQ(full_result, lean_result);
    EXPECT_EQ(full_result, 99);
}

TEST_F(result_lean_comparison_test_t, match_error_identical_between_modes) {
    auto full = result_t<int, false>::make_error(make_err(8), "err");
    auto lean = result_t<int, true>::make_error(make_err(8), "err");

    auto full_result = full.match([](const int&) { return 0; },
                                  [](const error_context_t& ctx) {
                                      return static_cast<int>(ctx.get_code().get_number());
                                  });
    auto lean_result = lean.match([](const int&) { return 0; },
                                  [](const error_context_t& ctx) {
                                      return static_cast<int>(ctx.get_code().get_number());
                                  });

    EXPECT_EQ(full_result, lean_result);
    EXPECT_EQ(full_result, 8);
}

TEST_F(result_lean_comparison_test_t, void_specialization_success_identical) {
    auto full = result_t<void, false>::make_success();
    auto lean = result_t<void, true>::make_success();

    EXPECT_EQ(full.is_success(), lean.is_success());
    EXPECT_EQ(static_cast<bool>(full), static_cast<bool>(lean));
}

TEST_F(result_lean_comparison_test_t, void_specialization_error_identical) {
    auto full = result_t<void, false>::make_error(make_err(99), "err");
    auto lean = result_t<void, true>::make_error(make_err(99), "err");

    EXPECT_EQ(full.is_error(), lean.is_error());
    EXPECT_EQ(full.error_code().get_number(), lean.error_code().get_number());
}

TEST_F(result_lean_comparison_test_t, make_error_from_context_extracts_code_in_both_modes) {
    error_context_t ctx{error_system::core::located_code_t{make_err(55)}, "源上下文"};
    ctx.with("k", "v");

    auto full = result_t<int, false>::make_error(ctx);
    auto lean = result_t<int, true>::make_error(ctx);

    EXPECT_EQ(full.error_code().get_code(), lean.error_code().get_code());
    EXPECT_EQ(full.error_code().get_code(), ctx.get_code().get_code());
}

TEST_F(result_lean_comparison_test_t, lean_size_not_larger_than_full) {
    static_assert(sizeof(result_t<int, true>) <= sizeof(result_t<int, false>),
                  "Lean result_t must not be larger than full result_t");
    static_assert(sizeof(result_t<std::string, true>) <= sizeof(result_t<std::string, false>),
                  "Lean result_t<string> must not be larger than full result_t<string>");
    static_assert(sizeof(result_t<void, true>) <= sizeof(result_t<void, false>),
                  "Lean result_t<void> must not be larger than full result_t<void>");
}

TEST_F(result_lean_comparison_test_t, operator_bool_identical_between_modes) {
    auto full_ok = result_t<int, false>::make_success(1);
    auto lean_ok = result_t<int, true>::make_success(1);
    auto full_err = result_t<int, false>::make_error(make_err(1), "e");
    auto lean_err = result_t<int, true>::make_error(make_err(1), "e");

    EXPECT_EQ(static_cast<bool>(full_ok), static_cast<bool>(lean_ok));
    EXPECT_EQ(static_cast<bool>(full_err), static_cast<bool>(lean_err));
    EXPECT_TRUE(static_cast<bool>(full_ok));
    EXPECT_FALSE(static_cast<bool>(full_err));
}

TEST_F(result_lean_comparison_test_t, value_or_default_identical_between_modes) {
    auto full = result_t<int, false>::make_error(make_err(1), "e");
    auto lean = result_t<int, true>::make_error(make_err(1), "e");

    EXPECT_EQ(full.value_or(999), lean.value_or(999));
    EXPECT_EQ(full.value_or(999), 999);
}
