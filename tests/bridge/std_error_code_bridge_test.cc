#include <cerrno>
#include <cstring>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "error_system/bridge/std_error_code_bridge.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"

using namespace error_system::bridge;
using error_system::core::code_t;
using error_system::core::error_code_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

/**
 * @file std_error_code_bridge_test.cc
 * @brief std::error_code / errno 桥接单元测试
 */

TEST(StdErrorCodeBridgeTest, success_code_maps_to_default_std_error_code) {
    const error_code_t success = error_code_t::make_success();
    const std::error_code ec = to_std_error_code(success);
    EXPECT_FALSE(ec);
    EXPECT_EQ(ec.value(), 0);
}

TEST(StdErrorCodeBridgeTest, error_code_round_trip_preserves_low_bits) {
    // 注意：std::error_code 的 value 是 int（通常 32 位），无法承载 64 位 identity。
    // level/system/subsystem/module 段位于高位，超出 int 范围会丢失。
    // 完整 64 位信息需保留 error_code_t，不可通过 std::error_code 传递。
    // 此测试仅验证低 16 位 number 字段的 round-trip 可逆性。
    const error_code_t original{error_level_t::error,
                                system_domain_t::system,
                                subsystem_id_t{0},
                                module_id_t{0},
                                error_number_t{42}};
    const std::error_code ec = to_std_error_code(original);
    ASSERT_TRUE(ec);
    EXPECT_EQ(&ec.category(), &error_system_category());

    const error_code_t restored = from_std_error_code(ec);
    EXPECT_TRUE(restored.is_error_code());
    EXPECT_EQ(restored.get_number(), 42);
}

TEST(StdErrorCodeBridgeTest, category_name_is_error_system) {
    const error_code_t code{error_level_t::error,
                            system_domain_t::system,
                            subsystem_id_t{0},
                            module_id_t{0},
                            error_number_t{1}};
    const std::error_code ec = to_std_error_code(code);
    EXPECT_STREQ(ec.category().name(), "error_system");
}

TEST(StdErrorCodeBridgeTest, category_message_returns_hex_form) {
    const error_code_t code{error_level_t::error,
                            system_domain_t::system,
                            subsystem_id_t{0},
                            module_id_t{0},
                            error_number_t{1}};
    const std::error_code ec = to_std_error_code(code);
    const std::string msg = ec.message();
    EXPECT_NE(msg.find("error_system:"), std::string::npos);
    EXPECT_NE(msg.find("0x"), std::string::npos);
}

TEST(StdErrorCodeBridgeTest, from_errno_marks_retryable_for_eagain) {
    const error_code_t code = from_errno(EAGAIN);
    EXPECT_TRUE(code.is_error_code());
    EXPECT_TRUE(code.is_retryable());
    EXPECT_TRUE(code.is_transient());
    EXPECT_EQ(code.get_system(), system_domain_t::system);
    EXPECT_EQ(code.get_number(), static_cast<uint16_t>(EAGAIN));
}

TEST(StdErrorCodeBridgeTest, from_errno_marks_retryable_for_etimedout) {
    const error_code_t code = from_errno(ETIMEDOUT);
    EXPECT_TRUE(code.is_retryable());
    EXPECT_TRUE(code.is_transient());
}

TEST(StdErrorCodeBridgeTest, from_errno_marks_retryable_for_eintr) {
    const error_code_t code = from_errno(EINTR);
    EXPECT_TRUE(code.is_retryable());
}

TEST(StdErrorCodeBridgeTest, from_errno_does_not_mark_non_retryable) {
    const error_code_t code = from_errno(EINVAL);
    EXPECT_FALSE(code.is_retryable());
    EXPECT_FALSE(code.is_transient());
    EXPECT_EQ(code.get_number(), static_cast<uint16_t>(EINVAL));
}

TEST(StdErrorCodeBridgeTest, to_errno_returns_zero_for_success) {
    EXPECT_EQ(to_errno(error_code_t::make_success()), 0);
}

TEST(StdErrorCodeBridgeTest, to_errno_round_trips_system_domain_errors) {
    const error_code_t code = from_errno(ENOENT);
    EXPECT_EQ(to_errno(code), ENOENT);
}

TEST(StdErrorCodeBridgeTest, to_errno_returns_eio_for_non_system_domain) {
    const error_code_t code{error_level_t::error,
                            system_domain_t::application,
                            subsystem_id_t{1},
                            module_id_t{1},
                            error_number_t{1}};
    EXPECT_EQ(to_errno(code), EIO);
}

TEST(StdErrorCodeBridgeTest, from_std_error_code_handles_generic_category) {
    const std::error_code ec{EINVAL, std::generic_category()};
    const error_code_t code = from_std_error_code(ec);
    EXPECT_TRUE(code.is_error_code());
    EXPECT_EQ(code.get_system(), system_domain_t::system);
    EXPECT_EQ(code.get_number(), static_cast<uint16_t>(EINVAL));
}

TEST(StdErrorCodeBridgeTest, from_std_error_code_handles_default_constructed) {
    const std::error_code ec{};
    const error_code_t code = from_std_error_code(ec);
    EXPECT_TRUE(code.is_success_code());
}

TEST(StdErrorCodeBridgeTest, from_std_error_code_handles_unknown_category) {
    // 自定义未知 category
    class unknown_category_t : public std::error_category {
    public:
        [[nodiscard]] const char* name() const noexcept override { return "unknown"; }
        [[nodiscard]] std::string message(int) const override { return "unknown"; }
    };
    static unknown_category_t cat{};
    const std::error_code ec{42, cat};
    const error_code_t code = from_std_error_code(ec);
    EXPECT_TRUE(code.is_error_code());
    EXPECT_EQ(code.get_system(), system_domain_t::system);
    EXPECT_EQ(code.get_number(), 42);
}
