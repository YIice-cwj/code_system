#include "error_system/utils/string_format.h"

#include <cstdint>

#include <string>

#include <gtest/gtest.h>

namespace error_system::utils {

    TEST(string_format_test, format_replaces_single_placeholder) {
        auto result = string_format_t::format("hello {}", "world");
        EXPECT_EQ(result, "hello world");
    }

    TEST(string_format_test, format_replaces_multiple_placeholders) {
        auto result = string_format_t::format("{} + {} = {}", 1, 2, 3);
        EXPECT_EQ(result, "1 + 2 = 3");
    }

    TEST(string_format_test, format_handles_integers) {
        auto result = string_format_t::format("number: {}", 42);
        EXPECT_EQ(result, "number: 42");
    }

    TEST(string_format_test, format_handles_floats) {
        auto result = string_format_t::format("pi: {}", 3.14);
        EXPECT_EQ(result, "pi: 3.14");
    }

    TEST(string_format_test, format_handles_no_placeholders) {
        auto result = string_format_t::format("no placeholders");
        EXPECT_EQ(result, "no placeholders");
    }

    TEST(string_format_test, format_handles_empty_string) {
        auto result = string_format_t::format("");
        EXPECT_TRUE(result.empty());
    }

    TEST(string_format_test, format_handles_more_placeholders_than_args) {
        auto result = string_format_t::format("{} {}", "only_one");
        EXPECT_EQ(result, "only_one {}");
    }

    TEST(string_format_test, format_handles_empty_arg) {
        auto result = string_format_t::format("start{}end", "");
        EXPECT_EQ(result, "startend");
    }

    TEST(string_format_test, format_escapes_double_braces) {
        auto result = string_format_t::format("{{hello}}", "world");
        EXPECT_EQ(result, "{hello}");
    }

    TEST(string_format_test, format_mixed_placeholders_and_escaped_braces) {
        auto result = string_format_t::format("{{}} {}", 42);
        EXPECT_EQ(result, "{} 42");
    }

    TEST(string_format_test, format_escapes_only_left_brace) {
        auto result = string_format_t::format("{{", "x");
        EXPECT_EQ(result, "{");
    }

    TEST(string_format_test, format_escapes_only_right_brace) {
        auto result = string_format_t::format("}}", "x");
        EXPECT_EQ(result, "}");
    }

    TEST(string_format_test, format_handles_bool) {
        EXPECT_EQ(string_format_t::format("{}", true), "true");
        EXPECT_EQ(string_format_t::format("{}", false), "false");
    }

    TEST(string_format_test, format_handles_char) {
        EXPECT_EQ(string_format_t::format("{}", 'A'), "A");
        EXPECT_EQ(string_format_t::format("{}", 'Z'), "Z");
    }

    TEST(string_format_test, format_handles_negative_int) {
        EXPECT_EQ(string_format_t::format("{}", -42), "-42");
        EXPECT_EQ(string_format_t::format("{}", -1), "-1");
    }

    /** Regression: nullptr 字面量曾匹配 string_view 分支触发 std::string_view(nullptr) UB（段错误） */
    TEST(string_format_test, format_handles_nullptr_literal) {
        EXPECT_EQ(string_format_t::format("{}", nullptr), "nullptr");
        EXPECT_EQ(string_format_t::format("val={}", nullptr), "val=nullptr");
    }

    /** Regression: null const char* 同样匹配 string_view 分支，指针分支的 null 检查成为死代码 */
    TEST(string_format_test, format_handles_null_char_pointer) {
        const char* null_str = nullptr;
        EXPECT_EQ(string_format_t::format("{}", null_str), "nullptr");
    }

    TEST(string_format_test, format_handles_null_object_pointer) {
        const int* null_ptr = nullptr;
        EXPECT_EQ(string_format_t::format("{}", null_ptr), "nullptr");
    }

    TEST(string_format_test, format_handles_non_null_object_pointer_as_hex_address) {
        int value = 42;
        const int* ptr = &value;
        auto result = string_format_t::format("{}", ptr);
        EXPECT_NE(result, "nullptr");
        EXPECT_EQ(result.substr(0, 2), "0x");
    }

}  // namespace error_system::utils
