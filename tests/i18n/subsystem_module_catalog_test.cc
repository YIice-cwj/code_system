/**
 * @file subsystem_module_catalog_test.cc
 * @brief subsystem_module_catalog_t 单元测试
 * @details 覆盖 register_subsystem_module 全部重载、get_subsystem_module_info 全部重载、
 *          locale 回退、clear、clear_locale、resolve_subsystem_module 接口实现、并发安全。
 */

#include <atomic>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "error_system/i18n/i_subsystem_module_resolver.h"
#include "error_system/i18n/locale.h"
#include "error_system/i18n/subsystem_module_catalog.h"

using error_system::i18n::i_subsystem_module_resolver_t;
using error_system::i18n::subsystem_module_catalog_t;
using error_system::i18n::subsystem_module_info_t;

namespace i18n = error_system::i18n;

class subsystem_module_catalog_test_t : public ::testing::Test {
protected:
    void SetUp() override { subsystem_module_catalog_t::instance().clear(); }

    void TearDown() override { subsystem_module_catalog_t::instance().clear(); }
};

TEST_F(subsystem_module_catalog_test_t, register_single_locale_query_hit) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 101, 1, "交易服务", "订单模块");
    auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::zh_CN, 101, 1);
    EXPECT_EQ(info.subsystem_name, "交易服务");
    EXPECT_EQ(info.module_name, "订单模块");
}

TEST_F(subsystem_module_catalog_test_t, register_default_locale_overload_hits_zh_CN) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        200, 5, "默认服务", "默认模块");
    auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::zh_CN, 200, 5);
    EXPECT_EQ(info.subsystem_name, "默认服务");
    EXPECT_EQ(info.module_name, "默认模块");
}

TEST_F(subsystem_module_catalog_test_t, query_miss_falls_back_to_default_zh_CN) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 300, 2, "中文服务", "中文模块");
    auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::en_US, 300, 2);
    EXPECT_EQ(info.subsystem_name, "中文服务");
    EXPECT_EQ(info.module_name, "中文模块");
}

TEST_F(subsystem_module_catalog_test_t, query_with_explicit_fallback_locale) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 400, 3, "中文服务", "中文模块");
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::en_US, 400, 3, "English Service", "English Module");
    auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::ja_JP, i18n::locale_t::en_US, 400, 3);
    EXPECT_EQ(info.subsystem_name, "English Service");
    EXPECT_EQ(info.module_name, "English Module");
}

TEST_F(subsystem_module_catalog_test_t, query_complete_miss_returns_default_strings) {
    auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::zh_CN, 999, 9);
    EXPECT_EQ(info.subsystem_name, "未知子系统");
    EXPECT_EQ(info.module_name, "未知模块");
}

TEST_F(subsystem_module_catalog_test_t, register_duplicate_keeps_first) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 500, 1, "首次注册", "首次模块");
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 500, 1, "二次注册", "二次模块");
    auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::zh_CN, 500, 1);
    EXPECT_EQ(info.subsystem_name, "首次注册");
    EXPECT_EQ(info.module_name, "首次模块");
}

TEST_F(subsystem_module_catalog_test_t, clear_removes_all_entries) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 600, 1, "服务A", "模块A");
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::en_US, 600, 1, "ServiceA", "ModuleA");
    subsystem_module_catalog_t::instance().clear();
    auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::zh_CN, 600, 1);
    EXPECT_EQ(info.subsystem_name, "未知子系统");
}

TEST_F(subsystem_module_catalog_test_t, clear_locale_removes_specific_locale) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 700, 1, "中文", "中文模块");
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::en_US, 700, 1, "English", "EnglishModule");
    const size_t removed = subsystem_module_catalog_t::instance().clear_locale(i18n::locale_t::en_US);
    EXPECT_EQ(removed, 1u);

    auto info_zh = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::zh_CN, 700, 1);
    EXPECT_EQ(info_zh.subsystem_name, "中文");
    auto info_en = subsystem_module_catalog_t::instance().get_subsystem_module_info(
        i18n::locale_t::en_US, 700, 1);
    EXPECT_EQ(info_en.subsystem_name, "中文");
}

TEST_F(subsystem_module_catalog_test_t, resolve_subsystem_module_interface_implementation) {
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 800, 1, "接口服务", "接口模块");
    const i_subsystem_module_resolver_t& resolver = subsystem_module_catalog_t::instance();
    auto info = resolver.resolve_subsystem_module(
        i18n::locale_t::zh_CN, i18n::locale_t::en_US, 800, 1);
    EXPECT_EQ(info.subsystem_name, "接口服务");
    EXPECT_EQ(info.module_name, "接口模块");
}

TEST_F(subsystem_module_catalog_test_t, concurrent_register_and_query_safe) {
    constexpr int N = 50;
    subsystem_module_catalog_t::instance().register_subsystem_module(
        i18n::locale_t::zh_CN, 900, 1, "并发服务", "并发模块");

    std::vector<std::thread> threads;
    std::atomic<int> hit_count{0};
    for (int t = 0; t < N; ++t) {
        threads.emplace_back([&] {
            auto info = subsystem_module_catalog_t::instance().get_subsystem_module_info(
                i18n::locale_t::zh_CN, 900, 1);
            if (info.subsystem_name == "并发服务") { ++hit_count; }
        });
    }
    for (auto& th : threads) { th.join(); }
    EXPECT_EQ(hit_count.load(), N);
}
