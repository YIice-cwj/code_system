#include "error_system/core/error_builder.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/core/result_t.h"
#include "error_system/domain/system_domain.h"

#include <gtest/gtest.h>

namespace error_system::core {

    class result_t_test : public ::testing::Test {};

    TEST_F(result_t_test, construct_with_value) {
        result_t<int> result(42);
        EXPECT_TRUE(result.is_success());
        EXPECT_FALSE(result.is_error());
        EXPECT_EQ(result.value(), 42);
    }

    TEST_F(result_t_test, construct_with_value_move) {
        std::string str = "hello";
        result_t<std::string> result(std::move(str));
        EXPECT_TRUE(result.is_success());
        EXPECT_EQ(result.value(), "hello");
    }

    TEST_F(result_t_test, construct_with_error_context) {
        auto code = error_builder_t::make_error_code(error_level_t::error, domain::system_domain_t::system, 1, 2, 100);
        error_context_t ctx(code, "test error");
        result_t<int> result(ctx);

        EXPECT_TRUE(result.is_error());
        EXPECT_FALSE(result.is_success());
        EXPECT_EQ(result.error().code.get_code(), code.get_code());
        EXPECT_EQ(result.error().message, "test error");
    }

    TEST_F(result_t_test, construct_with_code_and_message) {
        auto code = error_builder_t::make_error_code(error_level_t::warn, domain::system_domain_t::database, 0, 0, 50);
        result_t<int> result(code, "database warning");

        EXPECT_TRUE(result.is_error());
        EXPECT_EQ(result.error().code.get_number(), 50);
        EXPECT_EQ(result.error().message, "database warning");
    }

    TEST_F(result_t_test, construct_void_success) {
        result_t<void> result;
        EXPECT_TRUE(result.is_success());
        EXPECT_FALSE(result.is_error());
    }

    TEST_F(result_t_test, construct_void_with_error_context) {
        auto code = error_builder_t::make_error_code(error_level_t::fatal, domain::system_domain_t::kernel, 0, 0, 1);
        error_context_t ctx(code, "fatal kernel error");
        result_t<void> result(ctx);

        EXPECT_TRUE(result.is_error());
        EXPECT_FALSE(result.is_success());
        EXPECT_EQ(result.error().message, "fatal kernel error");
    }

    TEST_F(result_t_test, construct_void_with_code_and_message) {
        auto code = error_builder_t::make_error_code(error_level_t::error, domain::system_domain_t::network, 0, 0, 404);
        result_t<void> result(code, "not found");

        EXPECT_TRUE(result.is_error());
        EXPECT_EQ(result.error().code.get_number(), 404);
    }

    TEST_F(result_t_test, value_mutable_access) {
        result_t<int> result(10);
        result.value() = 20;
        EXPECT_EQ(result.value(), 20);
    }

    TEST_F(result_t_test, complex_value_type) {
        result_t<std::vector<int>> result(std::vector<int>{1, 2, 3});
        EXPECT_TRUE(result.is_success());
        EXPECT_EQ(result.value().size(), 3);
        EXPECT_EQ(result.value()[0], 1);
    }

    TEST_F(result_t_test, error_context_with_cause) {
        auto code1 = error_builder_t::make_error_code(error_level_t::error, domain::system_domain_t::system, 0, 0, 1);
        auto code2 =
            error_builder_t::make_error_code(error_level_t::error, domain::system_domain_t::middleware, 0, 0, 2);

        error_context_t cause(code1, "root cause");
        error_context_t ctx(code2, "wrapped error");
        ctx = ctx.wrap(cause);

        result_t<int> result(ctx);
        EXPECT_TRUE(result.is_error());
        EXPECT_NE(result.error().cause, nullptr);
        EXPECT_EQ(result.error().cause->message, "root cause");
    }

}  // namespace error_system::core
