#include "error_system/core/error_context.h"
#include "error_system/core/error_builder.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/i18n/json_translator.h"
#include "error_system/i18n/translator_registry.h"

#include <gtest/gtest.h>

namespace error_system::core {

    class error_context_test : public ::testing::Test {};

    TEST_F(error_context_test, default_construct_is_invalid) {
        error_context_t ctx;
        EXPECT_FALSE(static_cast<bool>(ctx));
        EXPECT_EQ(ctx.code.get_code(), 0);
        EXPECT_TRUE(ctx.message.empty());
        EXPECT_EQ(ctx.cause, nullptr);
    }

    TEST_F(error_context_test, construct_with_code_and_message) {
        auto code = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::system, 1, 2, 100);
        error_context_t ctx(code, "test message");

        EXPECT_TRUE(static_cast<bool>(ctx));
        EXPECT_EQ(ctx.code.get_level(), error_level_t::error);
        EXPECT_EQ(ctx.code.get_system(), domain::system_domain_t::system);
        EXPECT_EQ(ctx.code.get_subsys(), 1);
        EXPECT_EQ(ctx.code.get_module(), 2);
        EXPECT_EQ(ctx.code.get_number(), 100);
        EXPECT_EQ(ctx.message, "test message");
    }

    TEST_F(error_context_test, construct_with_empty_message) {
        auto code = error_builder_t::make_error_code(
            error_level_t::warn, domain::system_domain_t::database, 0, 0, 0);
        error_context_t ctx(code);

        EXPECT_TRUE(static_cast<bool>(ctx));
        EXPECT_TRUE(ctx.message.empty());
    }

    TEST_F(error_context_test, construct_with_zero_code_is_invalid) {
        error_context_t ctx(error_code_t{0}, "zero code");
        EXPECT_FALSE(static_cast<bool>(ctx));
    }

    TEST_F(error_context_test, wrap_creates_cause_chain) {
        auto root_code = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::kernel, 0, 0, 1);
        auto wrap_code = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::middleware, 0, 0, 2);

        error_context_t root_ctx(root_code, "root cause");
        error_context_t wrap_ctx(wrap_code, "wrapped error");

        error_context_t chained = wrap_ctx.wrap(root_ctx);

        EXPECT_TRUE(static_cast<bool>(chained));
        EXPECT_EQ(chained.message, "wrapped error");
        EXPECT_NE(chained.cause, nullptr);
        EXPECT_EQ(chained.cause->message, "root cause");
        EXPECT_EQ(chained.cause->code.get_system(), domain::system_domain_t::kernel);
    }

    TEST_F(error_context_test, wrap_preserves_original_unchanged) {
        auto code1 = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::system, 0, 0, 1);
        auto code2 = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::network, 0, 0, 2);

        error_context_t original(code1, "original");
        error_context_t wrapper(code2, "wrapper");

        error_context_t result = wrapper.wrap(original);

        EXPECT_EQ(original.cause, nullptr);
        EXPECT_EQ(original.message, "original");
    }

    TEST_F(error_context_test, to_string_with_cause_without_translator) {
        auto code1 = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::system, 0, 0, 1);
        auto code2 = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::middleware, 0, 0, 2);

        error_context_t root(code1, "root");
        error_context_t ctx(code2, "main");
        ctx = ctx.wrap(root);

        std::string str = ctx.to_string();
        EXPECT_NE(str.find("main"), std::string::npos);
        EXPECT_NE(str.find("root"), std::string::npos);
        EXPECT_NE(str.find("Caused by"), std::string::npos);
    }

    TEST_F(error_context_test, to_string_with_translator) {
        auto code = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::system, 0, 0, 0);
        error_context_t ctx(code, "translated error");

        i18n::json_translator_t translator(i18n::language_t::zh_cn);
        std::string str = ctx.to_string(&translator);

        EXPECT_NE(str.find("translated error"), std::string::npos);
    }

    TEST_F(error_context_test, to_string_uses_registry_translator) {
        auto code = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::system, 0, 0, 0);
        error_context_t ctx(code, "registry error");

        i18n::json_translator_t translator(i18n::language_t::en_us);
        i18n::translator_registry_t::instance().set(&translator);

        std::string str = ctx.to_string();
        EXPECT_NE(str.find("registry error"), std::string::npos);

        i18n::translator_registry_t::instance().set(nullptr);
    }

    TEST_F(error_context_test, to_string_deep_cause_chain) {
        auto code1 = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::system, 0, 0, 1);
        auto code2 = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::kernel, 0, 0, 2);
        auto code3 = error_builder_t::make_error_code(
            error_level_t::error, domain::system_domain_t::middleware, 0, 0, 3);

        error_context_t level1(code1, "level1");
        error_context_t level2(code2, "level2");
        error_context_t level3(code3, "level3");

        level2 = level2.wrap(level1);
        level3 = level3.wrap(level2);

        std::string str = level3.to_string();
        EXPECT_NE(str.find("level3"), std::string::npos);
        EXPECT_NE(str.find("level2"), std::string::npos);
        EXPECT_NE(str.find("level1"), std::string::npos);
    }

}  // namespace error_system::core
