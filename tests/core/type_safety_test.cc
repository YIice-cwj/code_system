#include <gtest/gtest.h>

#include <array>
#include <string>

#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/core/result.h"
#include "error_system/domain/system_domain.h"

using error_system::core::all_unique;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::module_id_t;
using error_system::core::result_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

namespace {
    constexpr error_code_t make_code(uint16_t number) {
        return error_code_t{error_level_t::error,
                            system_domain_t::application,
                            subsystem_id_t{1},
                            module_id_t{1},
                            error_number_t{number}};
    }
}

/**
 * @file type_safety_test.cc
 * @brief 类型安全改进测试
 * @details 验证编译期冲突检测工具与 make_error 防御性断言。
 */

TEST(AllUniqueTest, empty_array_is_unique) {
    constexpr std::array<error_code_t, 0> empty{};
    static_assert(all_unique(empty), "empty array should be unique");
    SUCCEED();
}

TEST(AllUniqueTest, single_element_is_unique) {
    constexpr std::array<error_code_t, 1> single = {error_code_t::make_success()};
    static_assert(all_unique(single), "single element should be unique");
    SUCCEED();
}

TEST(AllUniqueTest, distinct_codes_are_unique) {
    constexpr std::array<error_code_t, 3> codes = {
        error_code_t{error_level_t::error, system_domain_t::application,
                     subsystem_id_t{1}, module_id_t{1}, error_number_t{1}},
        error_code_t{error_level_t::error, system_domain_t::application,
                     subsystem_id_t{1}, module_id_t{1}, error_number_t{2}},
        error_code_t{error_level_t::error, system_domain_t::application,
                     subsystem_id_t{1}, module_id_t{1}, error_number_t{3}},
    };
    static_assert(all_unique(codes), "distinct codes should be unique");
    SUCCEED();
}

TEST(AllUniqueTest, duplicate_codes_detected) {
    constexpr std::array<error_code_t, 3> codes = {
        error_code_t{error_level_t::error, system_domain_t::application,
                     subsystem_id_t{1}, module_id_t{1}, error_number_t{1}},
        error_code_t{error_level_t::error, system_domain_t::application,
                     subsystem_id_t{1}, module_id_t{1}, error_number_t{2}},
        error_code_t{error_level_t::error, system_domain_t::application,
                     subsystem_id_t{1}, module_id_t{1}, error_number_t{1}},
    };
    EXPECT_FALSE(all_unique(codes));
}

TEST(AllUniqueTest, works_at_compile_time) {
    constexpr error_code_t code_a = make_code(1);
    constexpr error_code_t code_b = make_code(2);
    constexpr std::array<error_code_t, 2> codes = {code_a, code_b};
    constexpr bool result = all_unique(codes);
    static_assert(result, "should be unique at compile time");
    SUCCEED();
}

#ifndef NDEBUG
TEST(MakeErrorAssertTest, make_error_with_success_code_triggers_assert) {
    EXPECT_DEATH(
        {
            (void)result_t<int>::make_error(error_code_t::make_success(), "should fail");
        },
        "make_error called with a success code");
}

TEST(MakeErrorAssertTest, make_error_void_with_success_code_triggers_assert) {
    EXPECT_DEATH(
        {
            (void)result_t<void>::make_error(error_code_t::make_success(), "should fail");
        },
        "make_error called with a success code");
}
#endif

TEST(MakeErrorAssertTest, make_error_with_error_code_does_not_assert) {
    auto result = result_t<int>::make_error(make_code(1), "valid error");
    EXPECT_TRUE(result.is_error());
}
