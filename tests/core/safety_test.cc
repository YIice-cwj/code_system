#include <gtest/gtest.h>

#include <cstring>
#include <string>

#include "error_system/config/error_config.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/serializer/error_context_serializer.h"
#include "error_system/core/error_level.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/domain/system_domain.h"
#include "error_system/i18n/subsystem_module_catalog.h"

using error_system::config::feature_flags_t;
using error_system::config::formatter_config_t;
using error_system::config::i18n_config_t;
using error_system::core::duplicate_policy_t;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_context_serializer_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::located_code_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;

/**
 * @file safety_test.cc
 * @brief 安全改进测试
 * @details 验证 wrap() 自环检测与 from_binary 字符串长度防御上限。
 *          使用 fixture 在 SetUp 中注册错误码，避免 fill_validation_fields_ 改写消息与码。
 */
namespace {

    /**
     * @brief 构造测试用错误上下文
     * @details 使用已注册的 error_code（subsystem=1, module=1, number=1）。
     *          若 number 不为 1，调用方需自行注册对应码。
     */
    error_context_t make_context(const std::string& message, uint16_t number = 1) {
        return error_context_t{located_code_t{error_code_t{error_level_t::error,
                                 system_domain_t::application,
                                 subsystem_id_t{1},
                                 module_id_t{1},
                                 error_number_t{number}}}, message};
    }

}  // namespace

class safety_test_t : public ::testing::Test {
protected:
    void SetUp() override {
        auto& registry = error_registry_t::instance();
        registry.unregister_all();
        registry.set_duplicate_policy(duplicate_policy_t::skip);
        registry.set_duplicate_warn_callback(nullptr);

        auto& catalog = error_system::i18n::subsystem_module_catalog_t::instance();
        catalog.clear();

        formatter_config_t::set_custom_formatter(nullptr);
        i18n_config_t::set_enable_i18n(true);
#ifdef ERROR_SYSTEM_ENABLE_VALIDATION
        feature_flags_t::set_enable_validation(true);
#endif
#ifdef ERROR_SYSTEM_ENABLE_LOCATION
        feature_flags_t::set_enable_source_location(false);
#endif
#ifdef ERROR_SYSTEM_ENABLE_STACKTRACE
        feature_flags_t::set_enable_stacktrace(false);
#endif

        registered_code_ = error_code_t{error_level_t::error,
                                        system_domain_t::application,
                                        subsystem_id_t{1},
                                        module_id_t{1},
                                        error_number_t{1}};
        catalog.register_subsystem_module(registered_code_.get_subsys(),
                                          registered_code_.get_module(),
                                          "safety", "test");
        registry.register_error(registered_code_, "ERR_SAFETY_TEST", "Safety test error");
    }

    void TearDown() override {
        error_registry_t::instance().unregister_all();
        error_system::i18n::subsystem_module_catalog_t::instance().clear();
        formatter_config_t::set_custom_formatter(nullptr);
        i18n_config_t::set_enable_i18n(true);
#ifdef ERROR_SYSTEM_ENABLE_VALIDATION
        feature_flags_t::set_enable_validation(true);
#endif
#ifdef ERROR_SYSTEM_ENABLE_LOCATION
        feature_flags_t::set_enable_source_location(true);
#endif
#ifdef ERROR_SYSTEM_ENABLE_STACKTRACE
        feature_flags_t::set_enable_stacktrace(true);
#endif
    }

    error_code_t registered_code_{};
};

TEST_F(safety_test_t, self_wrap_does_not_create_cycle) {
    error_context_t outer = make_context("outer");
    error_context_t wrapped = outer.wrap(std::move(outer));
    EXPECT_EQ(wrapped.cause(), nullptr);
}

TEST_F(safety_test_t, normal_wrap_sets_cause) {
    error_context_t outer = make_context("outer");
    error_context_t inner = make_context("inner");
    error_context_t wrapped = outer.wrap(std::move(inner));
    ASSERT_NE(wrapped.cause(), nullptr);
    EXPECT_EQ(wrapped.cause()->get_message(), "inner");
}

TEST_F(safety_test_t, deep_cause_chain_wrap_succeeds) {
    error_context_t level_0 = make_context("level_0");
    error_context_t level_1 = make_context("level_1");
    error_context_t level_2 = make_context("level_2");
    error_context_t wrapped_1 = level_1.wrap(std::move(level_0));
    error_context_t wrapped_2 = level_2.wrap(std::move(wrapped_1));
    ASSERT_NE(wrapped_2.cause(), nullptr);
    EXPECT_EQ(wrapped_2.cause()->get_message(), "level_1");
    ASSERT_NE(wrapped_2.cause()->cause(), nullptr);
    EXPECT_EQ(wrapped_2.cause()->cause()->get_message(), "level_0");
}

TEST_F(safety_test_t, move_self_wrap_does_not_create_cycle) {
    error_context_t outer = make_context("outer");
    error_context_t moved_outer = std::move(outer);
    error_context_t wrapped = moved_outer.wrap(std::move(moved_outer));
    EXPECT_EQ(wrapped.cause(), nullptr);
}

TEST_F(safety_test_t, normal_roundtrip_succeeds) {
    error_context_t original = make_context("normal message");
    std::string binary = error_context_serializer_t::to_binary(original);
    auto result = error_context_serializer_t::from_binary(binary);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->get_message(), "normal message");
}

TEST_F(safety_test_t, oversized_string_length_rejected) {
    error_context_t original = make_context("test");
    std::string binary = error_context_serializer_t::to_binary(original);

    if (binary.size() >= 8) {
        const uint32_t fake_huge_length = 0x00FFFFFF;
        std::memcpy(&binary[4], &fake_huge_length, sizeof(fake_huge_length));
    }

    auto result = error_context_serializer_t::from_binary(binary);
    EXPECT_FALSE(result.has_value());
}

TEST_F(safety_test_t, truncated_input_rejected) {
    error_context_t original = make_context("test");
    std::string binary = error_context_serializer_t::to_binary(original);
    binary.resize(binary.size() / 2);
    auto result = error_context_serializer_t::from_binary(binary);
    EXPECT_FALSE(result.has_value());
}

TEST_F(safety_test_t, corrupted_magic_rejected) {
    error_context_t original = make_context("test");
    std::string binary = error_context_serializer_t::to_binary(original);
    if (!binary.empty()) {
        binary[0] = static_cast<char>(0xFF);
    }
    auto result = error_context_serializer_t::from_binary(binary);
    EXPECT_FALSE(result.has_value());
}
