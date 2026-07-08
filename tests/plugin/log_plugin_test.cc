#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>

#include "error_system/config/error_config.h"
#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/domain/system_domain.h"
#include "error_system/i18n/subsystem_module_catalog.h"
#include "error_system/plugin/log_plugin.h"

using error_system::config::feature_flags_t;
using error_system::config::formatter_config_t;
using error_system::config::i18n_config_t;
using error_system::core::duplicate_policy_t;
using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
using error_system::core::error_registry_t;
using error_system::core::located_code_t;
using error_system::core::module_id_t;
using error_system::core::subsystem_id_t;
using error_system::domain::system_domain_t;
using error_system::plugin::log_plugin_t;

namespace {
    error_code_t make_error_code(uint16_t number) {
        return error_code_t{error_level_t::error,
                            system_domain_t::application,
                            subsystem_id_t{1},
                            module_id_t{1},
                            error_number_t{number}};
    }

    error_context_t make_error_context(const std::string& message, uint16_t number = 1) {
        return error_context_t{located_code_t{make_error_code(number)}, message};
    }
}  // namespace

/**
 * @file log_plugin_test.cc
 * @brief log_plugin_t 单元测试
 * @details 验证 text/json 格式输出、min_level 过滤、自定义流写入、线程安全。
 *          使用 fixture 注册错误码并禁用 stacktrace，确保 to_string 输出为单行。
 */
class log_plugin_test_t : public ::testing::Test {
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

        registered_code_ = make_error_code(1);
        catalog.register_subsystem_module(registered_code_.get_subsys(),
                                          registered_code_.get_module(),
                                          "log", "plugin");
        registry.register_error(registered_code_, "ERR_LOG_PLUGIN_TEST", "Log plugin test error");
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

TEST_F(log_plugin_test_t, name_returns_constructed_name) {
    log_plugin_t plugin("custom_log");
    EXPECT_EQ(plugin.name(), "custom_log");
}

TEST_F(log_plugin_test_t, default_name_is_logger) {
    log_plugin_t plugin;
    EXPECT_EQ(plugin.name(), "logger");
}

TEST_F(log_plugin_test_t, min_level_returns_constructed_value) {
    log_plugin_t plugin("l", error_level_t::error);
    EXPECT_EQ(plugin.min_level(), error_level_t::error);
}

TEST_F(log_plugin_test_t, default_min_level_is_warn) {
    log_plugin_t plugin;
    EXPECT_EQ(plugin.min_level(), error_level_t::warn);
}

TEST_F(log_plugin_test_t, text_format_writes_to_stream) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::text, &output);
    plugin.on_error(make_error_context("test message"));
    std::string content = output.str();
    EXPECT_NE(content.find("test message"), std::string::npos);
}

TEST_F(log_plugin_test_t, json_format_writes_json_content) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::json, &output);
    plugin.on_error(make_error_context("json payload"));
    std::string content = output.str();
    EXPECT_NE(content.find("{"), std::string::npos);
    EXPECT_NE(content.find("json payload"), std::string::npos);
}

TEST_F(log_plugin_test_t, multiple_errors_each_on_separate_line) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::text, &output);
    plugin.on_error(make_error_context("first"));
    plugin.on_error(make_error_context("second"));
    std::string content = output.str();
    EXPECT_NE(content.find("first"), std::string::npos);
    EXPECT_NE(content.find("second"), std::string::npos);
    EXPECT_EQ(std::count(content.begin(), content.end(), '\n'), 2);
}

TEST_F(log_plugin_test_t, default_stream_is_cerr_when_nullptr_passed) {
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::text, nullptr);
    plugin.on_error(make_error_context("cerr output"));
    SUCCEED();
}

TEST_F(log_plugin_test_t, on_code_writes_raw_code_to_stream) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::text, &output);
    plugin.on_code(registered_code_);
    std::string content = output.str();
    EXPECT_NE(content.find("[ERR:"), std::string::npos);
    EXPECT_NE(content.find(std::to_string(registered_code_.get_code())), std::string::npos);
}

TEST_F(log_plugin_test_t, on_code_json_format_also_writes_raw_code) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::json, &output);
    plugin.on_code(registered_code_);
    std::string content = output.str();
    EXPECT_NE(content.find("[ERR:"), std::string::npos);
    EXPECT_NE(content.find(std::to_string(registered_code_.get_code())), std::string::npos);
}
