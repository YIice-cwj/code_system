#include <cstring>

#include <gtest/gtest.h>

#include "error_system/bridge/c_abi_export.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"

using error_system::core::code_t;
using error_system::core::error_code_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

/**
 * @file c_abi_export_test.cc
 * @brief C ABI 导出层单元测试
 */

TEST(CAbiExportTest, create_and_free_handle_round_trip) {
    const error_code_t original{error_level_t::error,
                                system_domain_t::application,
                                subsystem_id_t{100},
                                module_id_t{200},
                                error_number_t{300}};
    const void* handle = error_system_code_create(original.get_code());
    ASSERT_NE(handle, nullptr);

    EXPECT_EQ(error_system_code_raw(handle), original.get_code());
    EXPECT_EQ(error_system_code_is_error(handle), 1);
    EXPECT_EQ(error_system_code_is_success(handle), 0);
    EXPECT_EQ(error_system_code_level(handle), static_cast<uint8_t>(error_level_t::error));
    EXPECT_EQ(error_system_code_system(handle), static_cast<uint8_t>(system_domain_t::application));
    EXPECT_EQ(error_system_code_subsystem(handle), 100);
    EXPECT_EQ(error_system_code_module(handle), 200);
    EXPECT_EQ(error_system_code_number(handle), 300);

    error_system_code_handle_free(handle);
}

TEST(CAbiExportTest, success_code_handle_reports_success) {
    const error_code_t success = error_code_t::make_success();
    const void* handle = error_system_code_create(success.get_code());
    ASSERT_NE(handle, nullptr);

    EXPECT_EQ(error_system_code_is_success(handle), 1);
    EXPECT_EQ(error_system_code_is_error(handle), 0);

    error_system_code_handle_free(handle);
}

TEST(CAbiExportTest, null_handle_is_safe_no_op) {
    EXPECT_EQ(error_system_code_raw(nullptr), 0u);
    EXPECT_EQ(error_system_code_is_error(nullptr), 0);
    EXPECT_EQ(error_system_code_is_success(nullptr), 0);
    EXPECT_EQ(error_system_code_level(nullptr), 0u);
    EXPECT_EQ(error_system_code_system(nullptr), 0u);
    EXPECT_EQ(error_system_code_subsystem(nullptr), 0u);
    EXPECT_EQ(error_system_code_module(nullptr), 0u);
    EXPECT_EQ(error_system_code_number(nullptr), 0u);
    EXPECT_EQ(error_system_code_is_retryable(nullptr), 0);
    EXPECT_EQ(error_system_code_is_transient(nullptr), 0);

    // free nullptr 应安全
    error_system_code_handle_free(nullptr);
}

TEST(CAbiExportTest, retryable_and_transient_flags_read) {
    error_code_t code{error_level_t::error,
                      system_domain_t::middleware,
                      subsystem_id_t{1},
                      module_id_t{1},
                      error_number_t{1}};
    code.set_retryable(true);
    code.set_transient(true);

    const void* handle = error_system_code_create(code.get_code());
    ASSERT_NE(handle, nullptr);

    EXPECT_EQ(error_system_code_is_retryable(handle), 1);
    EXPECT_EQ(error_system_code_is_transient(handle), 1);

    error_system_code_handle_free(handle);
}

TEST(CAbiExportTest, message_writes_hex_form) {
    // level=error(3)、system=system(1)、number=1，sign=1(失败) → 0x8301000000000001
    const error_code_t code{error_level_t::error,
                            system_domain_t::system,
                            subsystem_id_t{0},
                            module_id_t{0},
                            error_number_t{1}};
    const void* handle = error_system_code_create(code.get_code());
    ASSERT_NE(handle, nullptr);

    char buf[64]{};
    const int written = error_system_code_message(handle, buf, sizeof(buf));
    EXPECT_EQ(written, 19);
    EXPECT_STREQ(buf, "0x8301000000000001");

    error_system_code_handle_free(handle);
}

TEST(CAbiExportTest, message_returns_zero_for_insufficient_buffer) {
    const error_code_t code{error_level_t::error,
                            system_domain_t::system,
                            subsystem_id_t{0},
                            module_id_t{0},
                            error_number_t{1}};
    const void* handle = error_system_code_create(code.get_code());
    ASSERT_NE(handle, nullptr);

    char small_buf[10]{};
    EXPECT_EQ(error_system_code_message(handle, small_buf, sizeof(small_buf)), 0);

    EXPECT_EQ(error_system_code_message(handle, nullptr, 64), 0);

    error_system_code_handle_free(handle);
}

TEST(CAbiExportTest, cpp_side_inline_functions_match_c_abi) {
    using error_system::abi::create_handle;
    using error_system::abi::free_handle;
    using error_system::abi::from_handle;
    using error_system::abi::get_raw;
    using error_system::abi::to_handle;

    const error_code_t original{error_level_t::warn,
                                system_domain_t::database,
                                subsystem_id_t{50},
                                module_id_t{60},
                                error_number_t{70}};
    auto cpp_handle = create_handle(original.get_code());
    ASSERT_NE(cpp_handle, nullptr);
    const void* opaque = from_handle(cpp_handle);
    EXPECT_EQ(to_handle(opaque), cpp_handle);
    EXPECT_EQ(get_raw(cpp_handle), original.get_code());

    free_handle(cpp_handle);
}
