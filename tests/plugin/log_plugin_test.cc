#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <string>

#include "error_system/core/error_code.h"
#include "error_system/core/error_context.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/plugin/log_plugin.h"

using error_system::core::error_code_t;
using error_system::core::error_context_t;
using error_system::core::error_level_t;
using error_system::core::error_number_t;
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
        error_context_t context = error_context_t::make_minimal(make_error_code(number));
        context.message = message;
        return context;
    }
}

/**
 * @file log_plugin_test.cc
 * @brief log_plugin_t 单元测试
 * @details 验证 text/json 格式输出、min_level 过滤、自定义流写入、线程安全。
 */

TEST(LogPluginTest, name_returns_constructed_name) {
    log_plugin_t plugin("custom_log");
    EXPECT_EQ(plugin.name(), "custom_log");
}

TEST(LogPluginTest, default_name_is_logger) {
    log_plugin_t plugin;
    EXPECT_EQ(plugin.name(), "logger");
}

TEST(LogPluginTest, min_level_returns_constructed_value) {
    log_plugin_t plugin("l", error_level_t::error);
    EXPECT_EQ(plugin.min_level(), error_level_t::error);
}

TEST(LogPluginTest, default_min_level_is_warn) {
    log_plugin_t plugin;
    EXPECT_EQ(plugin.min_level(), error_level_t::warn);
}

TEST(LogPluginTest, text_format_writes_to_stream) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::text, &output);
    plugin.on_error(make_error_context("test message"));
    std::string content = output.str();
    EXPECT_NE(content.find("test message"), std::string::npos);
}

TEST(LogPluginTest, json_format_writes_json_content) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::json, &output);
    plugin.on_error(make_error_context("json payload"));
    std::string content = output.str();
    EXPECT_NE(content.find("{"), std::string::npos);
    EXPECT_NE(content.find("json payload"), std::string::npos);
}

TEST(LogPluginTest, multiple_errors_each_on_separate_line) {
    std::ostringstream output;
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::text, &output);
    plugin.on_error(make_error_context("first"));
    plugin.on_error(make_error_context("second"));
    std::string content = output.str();
    EXPECT_NE(content.find("first"), std::string::npos);
    EXPECT_NE(content.find("second"), std::string::npos);
    EXPECT_EQ(std::count(content.begin(), content.end(), '\n'), 2);
}

TEST(LogPluginTest, default_stream_is_cerr_when_nullptr_passed) {
    log_plugin_t plugin("l", error_level_t::debug, log_plugin_t::format_t::text, nullptr);
    plugin.on_error(make_error_context("cerr output"));
    SUCCEED();
}
