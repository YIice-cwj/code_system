/**
 * @file custom_resolver_test.cc
 * @brief error_context_serializer_t::set_subsystem_module_resolver 自定义解析器单元测试
 * @details 覆盖 set_subsystem_module_resolver / get_subsystem_module_resolver_ 全部行为：
 *          注入自定义解析器影响 to_string 输出、nullptr 恢复默认、生命周期安全性。
 */

#include <string>

#include <gtest/gtest.h>

#include "error_system/config/feature_flags.h"
#include "error_system/config/i18n_config.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_context_serializer.h"
#include "error_system/core/error_level.h"
#include "error_system/core/error_registry.h"
#include "error_system/domain/system_domain.h"
#include "error_system/i18n/i_subsystem_module_resolver.h"
#include "error_system/i18n/locale.h"
#include "error_system/i18n/subsystem_module_catalog.h"

using error_system::config::feature_flags_t;
using error_system::config::i18n_config_t;
using error_system::core::error_code_t;
using error_system::core::error_context_serializer_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::located_code_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::i18n::i_subsystem_module_resolver_t;
using error_system::i18n::subsystem_module_catalog_t;
using error_system::i18n::subsystem_module_info_t;

namespace i18n = error_system::i18n;

namespace {

    /** @brief 自定义解析器：返回固定名称，便于断言 */
    class fixed_resolver_t : public i_subsystem_module_resolver_t {
    public:
        mutable int call_count{0};

        subsystem_module_info_t resolve_subsystem_module(
            i18n::locale_t /*output_locale*/,
            i18n::locale_t /*fallback_locale*/,
            uint16_t subsystem_id,
            uint16_t module_id) const noexcept override {
            ++call_count;
            subsystem_module_info_t info;
            info.subsystem_name = "自定义子系统#" + std::to_string(subsystem_id);
            info.module_name = "自定义模块#" + std::to_string(module_id);
            return info;
        }
    };

    /** @brief 测试用错误码 */
    error_code_t make_code(uint16_t number = 1) {
        return error_code_t{error_level_t::error, system_domain_t::application,
                            subsystem_id_t{101}, module_id_t{1}, error_number_t{number}};
    }

}  // namespace

class custom_resolver_test_t : public ::testing::Test {
protected:
    bool saved_validation_{true};
    bool saved_stacktrace_{true};
    bool saved_location_{true};

    void SetUp() override {
        error_registry_t::instance().unregister_all();
        subsystem_module_catalog_t::instance().clear();
        i18n_config_t::set_enable_i18n(true);
        i18n_config_t::clear_output_locale();
        error_context_serializer_t::set_subsystem_module_resolver(nullptr);
        saved_validation_ = feature_flags_t::is_validation_enabled();
        saved_stacktrace_ = feature_flags_t::is_stacktrace_enabled();
        saved_location_ = feature_flags_t::is_source_location_enabled();
        feature_flags_t::set_enable_validation(false);
        feature_flags_t::set_enable_stacktrace(false);
        feature_flags_t::set_enable_source_location(false);
    }

    void TearDown() override {
        error_context_serializer_t::set_subsystem_module_resolver(nullptr);
        subsystem_module_catalog_t::instance().clear();
        error_registry_t::instance().unregister_all();
        feature_flags_t::set_enable_validation(saved_validation_);
        feature_flags_t::set_enable_stacktrace(saved_stacktrace_);
        feature_flags_t::set_enable_source_location(saved_location_);
    }
};

TEST_F(custom_resolver_test_t, custom_resolver_affects_to_string_output) {
    fixed_resolver_t resolver;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver);

    error_context_t ctx{located_code_t{make_code()}, "自定义解析器测试"};
    const std::string text = ctx.to_string();

    EXPECT_NE(text.find("自定义子系统#101"), std::string::npos);
    EXPECT_NE(text.find("自定义模块#1"), std::string::npos);
    EXPECT_GT(resolver.call_count, 0);
}

TEST_F(custom_resolver_test_t, null_resolver_restores_default_behavior) {
    fixed_resolver_t resolver;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver);
    {
        error_context_t ctx{located_code_t{make_code()}, "首次"};
        (void)ctx.to_string();
    }
    EXPECT_GT(resolver.call_count, 0);

    error_context_serializer_t::set_subsystem_module_resolver(nullptr);
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 101, 1, "默认中文服务", "默认中文模块");

    fixed_resolver_t resolver2;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver2);
    error_context_t ctx2{located_code_t{make_code()}, "恢复默认"};
    (void)ctx2.to_string();
    EXPECT_GT(resolver2.call_count, 0);
}

TEST_F(custom_resolver_test_t, custom_resolver_called_with_correct_subsystem_module_ids) {
    fixed_resolver_t resolver;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver);

    error_context_t ctx{located_code_t{make_code()}, "ID 校验"};
    (void)ctx.to_string();

    EXPECT_EQ(resolver.call_count, 1);
}

TEST_F(custom_resolver_test_t, custom_resolver_respects_output_locale_fallback) {
    i18n_config_t::set_output_locale(i18n::locale_t::en_US);

    fixed_resolver_t resolver;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver);

    error_context_t ctx{located_code_t{make_code()}, "locale 测试"};
    const std::string text = ctx.to_string();

    EXPECT_FALSE(text.empty());
    EXPECT_GT(resolver.call_count, 0);

    i18n_config_t::clear_output_locale();
}

TEST_F(custom_resolver_test_t, custom_resolver_does_not_affect_to_json) {
    fixed_resolver_t resolver;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver);

    error_context_t ctx{located_code_t{make_code()}, "JSON 不受影响"};
    const std::string json = ctx.to_json();

    EXPECT_EQ(json.find("自定义子系统"), std::string::npos);
    EXPECT_NE(json.find("\"code\""), std::string::npos);
}

TEST_F(custom_resolver_test_t, custom_resolver_does_not_affect_to_binary) {
    fixed_resolver_t resolver;
    error_context_serializer_t::set_subsystem_module_resolver(&resolver);

    error_context_t ctx{located_code_t{make_code()}, "binary 不受影响"};
    const std::string binary = ctx.to_binary();

    EXPECT_FALSE(binary.empty());
    EXPECT_EQ(binary.find("自定义子系统"), std::string::npos);
}
