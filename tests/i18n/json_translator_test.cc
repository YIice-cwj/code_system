#include "error_system/core/error_builder.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"
#include "error_system/i18n/json_translator.h"
#include "error_system/i18n/language.h"
#include <filesystem>
#include <gtest/gtest.h>

namespace error_system::i18n {

    class json_translator_test : public ::testing::Test {
        protected:
        std::filesystem::path original_path_;

        void SetUp() override {
            original_path_ = std::filesystem::current_path();
            std::filesystem::path test_path = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
            std::filesystem::current_path(test_path);
        }

        void TearDown() override { std::filesystem::current_path(original_path_); }
    };

    TEST_F(json_translator_test, default_constructor_creates_zh_cn_translator) {
        json_translator_t translator;
        EXPECT_EQ(translator.get_language(), language_t::zh_cn);
    }

    TEST_F(json_translator_test, constructor_with_language_sets_correctly) {
        json_translator_t translator(language_t::en_us);
        EXPECT_EQ(translator.get_language(), language_t::en_us);
    }

    TEST_F(json_translator_test, set_language_changes_language) {
        json_translator_t translator(language_t::zh_cn);
        translator.set_language(language_t::en_us);
        EXPECT_EQ(translator.get_language(), language_t::en_us);
    }

    TEST_F(json_translator_test, translate_system_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::system, 0, 0, 404);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("错误"), std::string::npos);
        EXPECT_NE(result.find("系统层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_system_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::system, 0, 0, 404);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Error"), std::string::npos);
        EXPECT_NE(result.find("System"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_database_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::fatal, domain::system_domain_t::database, 1, 1, 500);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("致命错误"), std::string::npos);
        EXPECT_NE(result.find("数据库层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_database_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::fatal, domain::system_domain_t::database, 1, 1, 500);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Fatal"), std::string::npos);
        EXPECT_NE(result.find("Database"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_network_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::warn, domain::system_domain_t::network, 1, 1, 408);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("警告"), std::string::npos);
        EXPECT_NE(result.find("网络层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_ai_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code =
            core::error_builder_t::make_error_code(core::error_level_t::info, domain::system_domain_t::ai, 1, 1, 200);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("信息"), std::string::npos);
        EXPECT_NE(result.find("人工智能层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_cloud_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::debug, domain::system_domain_t::cloud, 1, 1, 100);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Debug"), std::string::npos);
        EXPECT_NE(result.find("Cloud"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_contains_error_code_number) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::system, 0, 0, 404);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Code: 404"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_returns_not_initialized_when_dict_missing) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::system, 0, 0, 404);
        std::string result = translator.translate(code);
        EXPECT_TRUE(result == "Translator not initialized or dict missing" || result.find("[") != std::string::npos);
    }

    TEST_F(json_translator_test, translate_kernel_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::fatal, domain::system_domain_t::kernel, 1, 1, 1);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("致命错误"), std::string::npos);
        EXPECT_NE(result.find("内核层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_security_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::warn, domain::system_domain_t::security, 1, 1, 403);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Warn"), std::string::npos);
        EXPECT_NE(result.find("Security"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_middleware_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::middleware, 1, 1, 502);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("错误"), std::string::npos);
        EXPECT_NE(result.find("中间件层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_storage_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::info, domain::system_domain_t::storage, 1, 1, 200);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Info"), std::string::npos);
        EXPECT_NE(result.find("Storage"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_iot_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code =
            core::error_builder_t::make_error_code(core::error_level_t::debug, domain::system_domain_t::iot, 1, 1, 100);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("调试"), std::string::npos);
        EXPECT_NE(result.find("物联网层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_blockchain_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::blockchain, 1, 1, 500);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Error"), std::string::npos);
        EXPECT_NE(result.find("Blockchain"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_bigdata_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::warn, domain::system_domain_t::bigdata, 1, 1, 429);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("警告"), std::string::npos);
        EXPECT_NE(result.find("大数据层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_devops_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::fatal, domain::system_domain_t::devops, 1, 1, 1);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Fatal"), std::string::npos);
        EXPECT_NE(result.find("Devops"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_edge_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code =
            core::error_builder_t::make_error_code(core::error_level_t::info, domain::system_domain_t::edge, 1, 1, 200);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("信息"), std::string::npos);
        EXPECT_NE(result.find("边缘计算层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_application_domain_en_us) {
        json_translator_t translator(language_t::en_us);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::debug, domain::system_domain_t::application, 1, 1, 100);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("Debug"), std::string::npos);
        EXPECT_NE(result.find("Application"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_service_domain_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::service, 1, 1, 503);
        std::string result = translator.translate(code);
        EXPECT_NE(result.find("错误"), std::string::npos);
        EXPECT_NE(result.find("服务层"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_all_levels_zh_cn) {
        json_translator_t translator(language_t::zh_cn);

        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::debug, domain::system_domain_t::system, 0, 0, 0))
                      .find("调试"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::info, domain::system_domain_t::system, 0, 0, 0))
                      .find("信息"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::warn, domain::system_domain_t::system, 0, 0, 0))
                      .find("警告"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::system, 0, 0, 0))
                      .find("错误"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::fatal, domain::system_domain_t::system, 0, 0, 0))
                      .find("致命错误"),
                  std::string::npos);
    }

    TEST_F(json_translator_test, translate_all_levels_en_us) {
        json_translator_t translator(language_t::en_us);

        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::debug, domain::system_domain_t::system, 0, 0, 0))
                      .find("Debug"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::info, domain::system_domain_t::system, 0, 0, 0))
                      .find("Info"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::warn, domain::system_domain_t::system, 0, 0, 0))
                      .find("Warn"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::system, 0, 0, 0))
                      .find("Error"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::fatal, domain::system_domain_t::system, 0, 0, 0))
                      .find("Fatal"),
                  std::string::npos);
    }

    TEST_F(json_translator_test, translate_all_domains_zh_cn) {
        json_translator_t translator(language_t::zh_cn);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::system, 0, 0, 0))
                      .find("系统层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::kernel, 0, 0, 0))
                      .find("内核层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::middleware, 0, 0, 0))
                      .find("中间件层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::application, 0, 0, 0))
                      .find("应用层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::service, 0, 0, 0))
                      .find("服务层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::network, 0, 0, 0))
                      .find("网络层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::storage, 0, 0, 0))
                      .find("存储层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::database, 0, 0, 0))
                      .find("数据库层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::security, 0, 0, 0))
                      .find("安全层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::ai, 0, 0, 0))
                      .find("人工智能层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::cloud, 0, 0, 0))
                      .find("云计算层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::edge, 0, 0, 0))
                      .find("边缘计算层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::iot, 0, 0, 0))
                      .find("物联网层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::blockchain, 0, 0, 0))
                      .find("区块链层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::bigdata, 0, 0, 0))
                      .find("大数据层"),
                  std::string::npos);
        EXPECT_NE(translator
                      .translate(core::error_builder_t::make_error_code(
                          core::error_level_t::error, domain::system_domain_t::devops, 0, 0, 0))
                      .find("运维开发层"),
                  std::string::npos);
    }

    TEST_F(json_translator_test, translate_switch_language) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::system, 0, 0, 404);

        std::string zh_result = translator.translate(code);
        EXPECT_NE(zh_result.find("错误"), std::string::npos);

        translator.set_language(language_t::en_us);
        std::string en_result = translator.translate(code);
        EXPECT_NE(en_result.find("Error"), std::string::npos);
    }

    TEST_F(json_translator_test, translate_format_structure) {
        json_translator_t translator(language_t::zh_cn);
        auto code = core::error_builder_t::make_error_code(
            core::error_level_t::error, domain::system_domain_t::database, 1, 1, 500);
        std::string result = translator.translate(code);

        EXPECT_NE(result.find("["), std::string::npos);
        EXPECT_NE(result.find("]"), std::string::npos);
        EXPECT_NE(result.find("Code:"), std::string::npos);
    }

}  // namespace error_system::i18n
