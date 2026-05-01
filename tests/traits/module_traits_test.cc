#include "error_system/traits/module_traits.h"

#include "error_system/module/ai_module.h"
#include "error_system/module/application_module.h"
#include "error_system/module/bigdata_module.h"
#include "error_system/module/blockchain_module.h"
#include "error_system/module/cloud_module.h"
#include "error_system/module/common_module.h"
#include "error_system/module/database_module.h"
#include "error_system/module/devops_module.h"
#include "error_system/module/edge_module.h"
#include "error_system/module/iot_module.h"
#include "error_system/module/kernel_module.h"
#include "error_system/module/middleware_module.h"
#include "error_system/module/network_module.h"
#include "error_system/module/security_module.h"
#include "error_system/module/service_module.h"
#include "error_system/module/storage_module.h"
#include "error_system/module/system_module.h"
#include "error_system/traits/module/ai_module_traits.h"
#include "error_system/traits/module/application_module_traits.h"
#include "error_system/traits/module/bigdata_module_traits.h"
#include "error_system/traits/module/blockchain_module_traits.h"
#include "error_system/traits/module/cloud_module_traits.h"
#include "error_system/traits/module/common_module_traits.h"
#include "error_system/traits/module/database_module_traits.h"
#include "error_system/traits/module/devops_module_traits.h"
#include "error_system/traits/module/edge_module_traits.h"
#include "error_system/traits/module/iot_module_traits.h"
#include "error_system/traits/module/kernel_module_traits.h"
#include "error_system/traits/module/middleware_module_traits.h"
#include "error_system/traits/module/network_module_traits.h"
#include "error_system/traits/module/security_module_traits.h"
#include "error_system/traits/module/service_module_traits.h"
#include "error_system/traits/module/storage_module_traits.h"
#include "error_system/traits/module/system_module_traits.h"

#include <gtest/gtest.h>

namespace error_system::traits {

    class module_traits_test : public ::testing::Test {};

    TEST_F(module_traits_test, system_module_to_int) {
        EXPECT_EQ(module_traits<module::system_module_t>::to_int(module::system_module_t::none), 0x0000);
        EXPECT_EQ(module_traits<module::system_module_t>::to_int(module::system_module_t::process), 0x0001);
        EXPECT_EQ(module_traits<module::system_module_t>::to_int(module::system_module_t::plugin), 0x0011);
    }

    TEST_F(module_traits_test, system_module_is_valid) {
        EXPECT_TRUE(module_traits<module::system_module_t>::is_valid(0x0000));
        EXPECT_TRUE(module_traits<module::system_module_t>::is_valid(0x0011));
        EXPECT_FALSE(module_traits<module::system_module_t>::is_valid(0x00FF));
    }

    TEST_F(module_traits_test, system_module_from_int) {
        EXPECT_EQ(module_traits<module::system_module_t>::from_int(0x0000), module::system_module_t::none);
        EXPECT_EQ(module_traits<module::system_module_t>::from_int(0x0001), module::system_module_t::process);
        EXPECT_EQ(module_traits<module::system_module_t>::from_int(0x00FF), module::system_module_t::none);
    }

    TEST_F(module_traits_test, system_module_to_string) {
        EXPECT_STREQ(module_traits<module::system_module_t>::to_string(module::system_module_t::none), "none");
        EXPECT_STREQ(module_traits<module::system_module_t>::to_string(module::system_module_t::thread), "thread");
        EXPECT_STREQ(module_traits<module::system_module_t>::to_string(module::system_module_t::plugin), "plugin");
    }

    TEST_F(module_traits_test, system_module_from_string) {
        EXPECT_EQ(module_traits<module::system_module_t>::from_string("none"), module::system_module_t::none);
        EXPECT_EQ(module_traits<module::system_module_t>::from_string("memory"), module::system_module_t::memory);
        EXPECT_EQ(module_traits<module::system_module_t>::from_string("unknown"), module::system_module_t::none);
    }

    TEST_F(module_traits_test, kernel_module_roundtrip) {
        using traits = module_traits<module::kernel_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::kernel_module_t::scheduler)),
                  module::kernel_module_t::scheduler);
        EXPECT_EQ(traits::from_string(traits::to_string(module::kernel_module_t::ebpf)), module::kernel_module_t::ebpf);
    }

    TEST_F(module_traits_test, network_module_roundtrip) {
        using traits = module_traits<module::network_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::network_module_t::socket)), module::network_module_t::socket);
        EXPECT_EQ(traits::from_string(traits::to_string(module::network_module_t::ssl_context)),
                  module::network_module_t::ssl_context);
    }

    TEST_F(module_traits_test, database_module_roundtrip) {
        using traits = module_traits<module::database_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::database_module_t::connection)),
                  module::database_module_t::connection);
        EXPECT_EQ(traits::from_string(traits::to_string(module::database_module_t::transaction)),
                  module::database_module_t::transaction);
    }

    TEST_F(module_traits_test, middleware_module_roundtrip) {
        using traits = module_traits<module::middleware_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::middleware_module_t::rpc_client)),
                  module::middleware_module_t::rpc_client);
        EXPECT_EQ(traits::from_string(traits::to_string(module::middleware_module_t::circuit_breaker)),
                  module::middleware_module_t::circuit_breaker);
    }

    TEST_F(module_traits_test, application_module_roundtrip) {
        using traits = module_traits<module::application_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::application_module_t::controller)),
                  module::application_module_t::controller);
        EXPECT_EQ(traits::from_string(traits::to_string(module::application_module_t::validator)),
                  module::application_module_t::validator);
    }

    TEST_F(module_traits_test, service_module_roundtrip) {
        using traits = module_traits<module::service_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::service_module_t::order_manager)),
                  module::service_module_t::order_manager);
        EXPECT_EQ(traits::from_string(traits::to_string(module::service_module_t::auth_manager)),
                  module::service_module_t::auth_manager);
    }

    TEST_F(module_traits_test, storage_module_roundtrip) {
        using traits = module_traits<module::storage_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::storage_module_t::file_reader)),
                  module::storage_module_t::file_reader);
        EXPECT_EQ(traits::from_string(traits::to_string(module::storage_module_t::bucket)),
                  module::storage_module_t::bucket);
    }

    TEST_F(module_traits_test, security_module_roundtrip) {
        using traits = module_traits<module::security_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::security_module_t::authenticator)),
                  module::security_module_t::authenticator);
        EXPECT_EQ(traits::from_string(traits::to_string(module::security_module_t::encryptor)),
                  module::security_module_t::encryptor);
    }

    TEST_F(module_traits_test, ai_module_roundtrip) {
        using traits = module_traits<module::ai_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::ai_module_t::model_loader)),
                  module::ai_module_t::model_loader);
        EXPECT_EQ(traits::from_string(traits::to_string(module::ai_module_t::inference_engine)),
                  module::ai_module_t::inference_engine);
    }

    TEST_F(module_traits_test, cloud_module_roundtrip) {
        using traits = module_traits<module::cloud_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::cloud_module_t::instance_manager)),
                  module::cloud_module_t::instance_manager);
        EXPECT_EQ(traits::from_string(traits::to_string(module::cloud_module_t::scaler)),
                  module::cloud_module_t::scaler);
    }

    TEST_F(module_traits_test, edge_module_roundtrip) {
        using traits = module_traits<module::edge_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::edge_module_t::gateway_controller)),
                  module::edge_module_t::gateway_controller);
        EXPECT_EQ(traits::from_string(traits::to_string(module::edge_module_t::sync_engine)),
                  module::edge_module_t::sync_engine);
    }

    TEST_F(module_traits_test, iot_module_roundtrip) {
        using traits = module_traits<module::iot_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::iot_module_t::device_registry)),
                  module::iot_module_t::device_registry);
        EXPECT_EQ(traits::from_string(traits::to_string(module::iot_module_t::telemetry_collector)),
                  module::iot_module_t::telemetry_collector);
    }

    TEST_F(module_traits_test, blockchain_module_roundtrip) {
        using traits = module_traits<module::blockchain_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::blockchain_module_t::consensus_engine)),
                  module::blockchain_module_t::consensus_engine);
        EXPECT_EQ(traits::from_string(traits::to_string(module::blockchain_module_t::contract_executor)),
                  module::blockchain_module_t::contract_executor);
    }

    TEST_F(module_traits_test, bigdata_module_roundtrip) {
        using traits = module_traits<module::bigdata_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::bigdata_module_t::data_collector)),
                  module::bigdata_module_t::data_collector);
        EXPECT_EQ(traits::from_string(traits::to_string(module::bigdata_module_t::query_engine)),
                  module::bigdata_module_t::query_engine);
    }

    TEST_F(module_traits_test, devops_module_roundtrip) {
        using traits = module_traits<module::devops_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::devops_module_t::build_executor)),
                  module::devops_module_t::build_executor);
        EXPECT_EQ(traits::from_string(traits::to_string(module::devops_module_t::deploy_engine)),
                  module::devops_module_t::deploy_engine);
    }

    TEST_F(module_traits_test, common_module_roundtrip) {
        using traits = module_traits<module::common_module_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(module::common_module_t::connector)),
                  module::common_module_t::connector);
        EXPECT_EQ(traits::from_string(traits::to_string(module::common_module_t::circuit_breaker)),
                  module::common_module_t::circuit_breaker);
    }

    TEST_F(module_traits_test, all_module_none_values) {
        EXPECT_EQ(module_traits<module::system_module_t>::from_int(0x0000), module::system_module_t::none);
        EXPECT_EQ(module_traits<module::kernel_module_t>::from_int(0x0100), module::kernel_module_t::none);
        EXPECT_EQ(module_traits<module::middleware_module_t>::from_int(0x0200), module::middleware_module_t::none);
        EXPECT_EQ(module_traits<module::application_module_t>::from_int(0x0300), module::application_module_t::none);
        EXPECT_EQ(module_traits<module::service_module_t>::from_int(0x0400), module::service_module_t::none);
        EXPECT_EQ(module_traits<module::network_module_t>::from_int(0x0500), module::network_module_t::none);
        EXPECT_EQ(module_traits<module::storage_module_t>::from_int(0x0600), module::storage_module_t::none);
        EXPECT_EQ(module_traits<module::database_module_t>::from_int(0x0700), module::database_module_t::none);
        EXPECT_EQ(module_traits<module::security_module_t>::from_int(0x0800), module::security_module_t::none);
        EXPECT_EQ(module_traits<module::ai_module_t>::from_int(0x0900), module::ai_module_t::none);
        EXPECT_EQ(module_traits<module::cloud_module_t>::from_int(0x0A00), module::cloud_module_t::none);
        EXPECT_EQ(module_traits<module::edge_module_t>::from_int(0x0B00), module::edge_module_t::none);
        EXPECT_EQ(module_traits<module::iot_module_t>::from_int(0x0C00), module::iot_module_t::none);
        EXPECT_EQ(module_traits<module::blockchain_module_t>::from_int(0x0D00), module::blockchain_module_t::none);
        EXPECT_EQ(module_traits<module::bigdata_module_t>::from_int(0x0E00), module::bigdata_module_t::none);
        EXPECT_EQ(module_traits<module::devops_module_t>::from_int(0x0F00), module::devops_module_t::none);
        EXPECT_EQ(module_traits<module::common_module_t>::from_int(0x0800), module::common_module_t::none);
    }

    TEST_F(module_traits_test, invalid_value_returns_none) {
        EXPECT_EQ(module_traits<module::system_module_t>::from_int(0x9999), module::system_module_t::none);
        EXPECT_EQ(module_traits<module::database_module_t>::from_string("not_exist"), module::database_module_t::none);
    }

    TEST_F(module_traits_test, is_valid_boundary) {
        EXPECT_TRUE(module_traits<module::system_module_t>::is_valid(0x0000));
        EXPECT_TRUE(module_traits<module::system_module_t>::is_valid(0x0011));
        EXPECT_FALSE(module_traits<module::system_module_t>::is_valid(0x0012));
        EXPECT_FALSE(module_traits<module::system_module_t>::is_valid(0xFFFF));
    }

    TEST_F(module_traits_test, to_string_unknown) {
        EXPECT_STREQ(module_traits<module::system_module_t>::to_string(static_cast<module::system_module_t>(0x9999)),
                     "unknown");
    }

}  // namespace error_system::traits
