#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_context_serializer.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"

using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_context_serializer_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

namespace {
    error_context_t make_context(const std::string& message, uint16_t number = 1) {
        error_context_t context = error_context_t::make_minimal(
            error_code_t{error_level_t::error,
                         system_domain_t::application,
                         subsystem_id_t{1},
                         module_id_t{1},
                         error_number_t{number}});
        context.message = message;
        return context;
    }
}

/**
 * @file safety_test.cc
 * @brief 安全改进测试
 * @details 验证 wrap() 自环检测与 from_binary 字符串长度防御上限。
 */

TEST(WrapCycleTest, self_wrap_does_not_create_cycle) {
    error_context_t outer = make_context("outer");
    error_context_t wrapped = outer.wrap(outer);
    EXPECT_EQ(wrapped.cause, nullptr);
}

TEST(WrapCycleTest, normal_wrap_sets_cause) {
    error_context_t outer = make_context("outer");
    error_context_t inner = make_context("inner");
    error_context_t wrapped = outer.wrap(inner);
    ASSERT_NE(wrapped.cause, nullptr);
    EXPECT_EQ(wrapped.cause->message, "inner");
}

TEST(WrapCycleTest, deep_cause_chain_wrap_succeeds) {
    error_context_t level_0 = make_context("level_0");
    error_context_t level_1 = make_context("level_1");
    error_context_t level_2 = make_context("level_2");
    error_context_t wrapped_1 = level_1.wrap(level_0);
    error_context_t wrapped_2 = level_2.wrap(wrapped_1);
    ASSERT_NE(wrapped_2.cause, nullptr);
    EXPECT_EQ(wrapped_2.cause->message, "level_1");
    ASSERT_NE(wrapped_2.cause->cause, nullptr);
    EXPECT_EQ(wrapped_2.cause->cause->message, "level_0");
}

TEST(WrapCycleTest, move_self_wrap_does_not_create_cycle) {
    error_context_t outer = make_context("outer");
    error_context_t moved_outer = std::move(outer);
    error_context_t wrapped = moved_outer.wrap(std::move(moved_outer));
    EXPECT_EQ(wrapped.cause, nullptr);
}

TEST(FromBinarySafetyTest, normal_roundtrip_succeeds) {
    error_context_t original = make_context("normal message");
    std::string binary = error_context_serializer_t::to_binary(original);
    auto result = error_context_serializer_t::from_binary(binary);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->message, "normal message");
}

TEST(FromBinarySafetyTest, oversized_string_length_rejected) {
    error_context_t original = make_context("test");
    std::string binary = error_context_serializer_t::to_binary(original);

    if (binary.size() >= 8) {
        const uint32_t fake_huge_length = 0x00FFFFFF;
        std::memcpy(&binary[4], &fake_huge_length, sizeof(fake_huge_length));
    }

    auto result = error_context_serializer_t::from_binary(binary);
    EXPECT_FALSE(result.has_value());
}

TEST(FromBinarySafetyTest, truncated_input_rejected) {
    error_context_t original = make_context("test");
    std::string binary = error_context_serializer_t::to_binary(original);
    binary.resize(binary.size() / 2);
    auto result = error_context_serializer_t::from_binary(binary);
    EXPECT_FALSE(result.has_value());
}

TEST(FromBinarySafetyTest, corrupted_magic_rejected) {
    error_context_t original = make_context("test");
    std::string binary = error_context_serializer_t::to_binary(original);
    if (!binary.empty()) {
        binary[0] = static_cast<char>(0xFF);
    }
    auto result = error_context_serializer_t::from_binary(binary);
    EXPECT_FALSE(result.has_value());
}
