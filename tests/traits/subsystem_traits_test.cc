#include "error_system/traits/subsystem_traits.h"

#include "error_system/subsystem/ai_subsystem.h"
#include "error_system/subsystem/application_subsystem.h"
#include "error_system/subsystem/bigdata_subsystem.h"
#include "error_system/subsystem/blockchain_subsystem.h"
#include "error_system/subsystem/cloud_subsystem.h"
#include "error_system/subsystem/database_subsystem.h"
#include "error_system/subsystem/devops_subsystem.h"
#include "error_system/subsystem/edge_subsystem.h"
#include "error_system/subsystem/iot_subsystem.h"
#include "error_system/subsystem/kernel_subsystem.h"
#include "error_system/subsystem/middleware_subsystem.h"
#include "error_system/subsystem/network_subsystem.h"
#include "error_system/subsystem/security_subsystem.h"
#include "error_system/subsystem/service_subsystem.h"
#include "error_system/subsystem/storage_subsystem.h"
#include "error_system/subsystem/system_subsystem.h"
#include "error_system/traits/subsystem/ai_subsystem_traits.h"
#include "error_system/traits/subsystem/application_subsystem_traits.h"
#include "error_system/traits/subsystem/bigdata_subsystem_traits.h"
#include "error_system/traits/subsystem/blockchain_subsystem_traits.h"
#include "error_system/traits/subsystem/cloud_subsystem_traits.h"
#include "error_system/traits/subsystem/database_subsystem_traits.h"
#include "error_system/traits/subsystem/devops_subsystem_traits.h"
#include "error_system/traits/subsystem/edge_subsystem_traits.h"
#include "error_system/traits/subsystem/iot_subsystem_traits.h"
#include "error_system/traits/subsystem/kernel_subsystem_traits.h"
#include "error_system/traits/subsystem/middleware_subsystem_traits.h"
#include "error_system/traits/subsystem/network_subsystem_traits.h"
#include "error_system/traits/subsystem/security_subsystem_traits.h"
#include "error_system/traits/subsystem/service_subsystem_traits.h"
#include "error_system/traits/subsystem/storage_subsystem_traits.h"
#include "error_system/traits/subsystem/system_subsystem_traits.h"

#include <gtest/gtest.h>

namespace error_system::traits {

    class subsystem_traits_test : public ::testing::Test {};

    TEST_F(subsystem_traits_test, system_subsystem_to_int) {
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::to_int(subsystem::system_subsystem_t::none), 0x0000);
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::to_int(subsystem::system_subsystem_t::process),
                  0x0001);
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::to_int(subsystem::system_subsystem_t::plugin),
                  0x000D);
    }

    TEST_F(subsystem_traits_test, system_subsystem_is_valid) {
        EXPECT_TRUE(subsystem_traits<subsystem::system_subsystem_t>::is_valid(0x0000));
        EXPECT_TRUE(subsystem_traits<subsystem::system_subsystem_t>::is_valid(0x000D));
        EXPECT_FALSE(subsystem_traits<subsystem::system_subsystem_t>::is_valid(0x00FF));
    }

    TEST_F(subsystem_traits_test, system_subsystem_from_int) {
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_int(0x0000),
                  subsystem::system_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_int(0x0001),
                  subsystem::system_subsystem_t::process);
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_int(0x00FF),
                  subsystem::system_subsystem_t::none);
    }

    TEST_F(subsystem_traits_test, system_subsystem_to_string) {
        EXPECT_STREQ(subsystem_traits<subsystem::system_subsystem_t>::to_string(subsystem::system_subsystem_t::none),
                     "none");
        EXPECT_STREQ(subsystem_traits<subsystem::system_subsystem_t>::to_string(subsystem::system_subsystem_t::process),
                     "process");
        EXPECT_STREQ(subsystem_traits<subsystem::system_subsystem_t>::to_string(subsystem::system_subsystem_t::plugin),
                     "plugin");
    }

    TEST_F(subsystem_traits_test, system_subsystem_from_string) {
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_string("none"),
                  subsystem::system_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_string("process"),
                  subsystem::system_subsystem_t::process);
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_string("unknown"),
                  subsystem::system_subsystem_t::none);
    }

    TEST_F(subsystem_traits_test, kernel_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::kernel_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::kernel_subsystem_t::scheduler)),
                  subsystem::kernel_subsystem_t::scheduler);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::kernel_subsystem_t::ebpf)),
                  subsystem::kernel_subsystem_t::ebpf);
    }

    TEST_F(subsystem_traits_test, network_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::network_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::network_subsystem_t::http)),
                  subsystem::network_subsystem_t::http);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::network_subsystem_t::mqtt)),
                  subsystem::network_subsystem_t::mqtt);
    }

    TEST_F(subsystem_traits_test, database_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::database_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::database_subsystem_t::mysql)),
                  subsystem::database_subsystem_t::mysql);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::database_subsystem_t::redis)),
                  subsystem::database_subsystem_t::redis);
    }

    TEST_F(subsystem_traits_test, middleware_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::middleware_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::middleware_subsystem_t::kafka)),
                  subsystem::middleware_subsystem_t::kafka);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::middleware_subsystem_t::message_queue)),
                  subsystem::middleware_subsystem_t::message_queue);
    }

    TEST_F(subsystem_traits_test, application_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::application_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::application_subsystem_t::web)),
                  subsystem::application_subsystem_t::web);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::application_subsystem_t::ai)),
                  subsystem::application_subsystem_t::ai);
    }

    TEST_F(subsystem_traits_test, service_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::service_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::service_subsystem_t::payment)),
                  subsystem::service_subsystem_t::payment);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::service_subsystem_t::auth)),
                  subsystem::service_subsystem_t::auth);
    }

    TEST_F(subsystem_traits_test, storage_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::storage_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::storage_subsystem_t::s3)),
                  subsystem::storage_subsystem_t::s3);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::storage_subsystem_t::ceph)),
                  subsystem::storage_subsystem_t::ceph);
    }

    TEST_F(subsystem_traits_test, security_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::security_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::security_subsystem_t::encryption)),
                  subsystem::security_subsystem_t::encryption);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::security_subsystem_t::firewall)),
                  subsystem::security_subsystem_t::firewall);
    }

    TEST_F(subsystem_traits_test, ai_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::ai_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::ai_subsystem_t::llm)), subsystem::ai_subsystem_t::llm);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::ai_subsystem_t::rag)),
                  subsystem::ai_subsystem_t::rag);
    }

    TEST_F(subsystem_traits_test, cloud_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::cloud_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::cloud_subsystem_t::kubernetes)),
                  subsystem::cloud_subsystem_t::kubernetes);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::cloud_subsystem_t::serverless)),
                  subsystem::cloud_subsystem_t::serverless);
    }

    TEST_F(subsystem_traits_test, edge_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::edge_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::edge_subsystem_t::gateway)),
                  subsystem::edge_subsystem_t::gateway);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::edge_subsystem_t::ai_inference)),
                  subsystem::edge_subsystem_t::ai_inference);
    }

    TEST_F(subsystem_traits_test, iot_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::iot_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::iot_subsystem_t::sensor)),
                  subsystem::iot_subsystem_t::sensor);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::iot_subsystem_t::digital_twin)),
                  subsystem::iot_subsystem_t::digital_twin);
    }

    TEST_F(subsystem_traits_test, blockchain_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::blockchain_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::blockchain_subsystem_t::smart_contract)),
                  subsystem::blockchain_subsystem_t::smart_contract);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::blockchain_subsystem_t::consensus)),
                  subsystem::blockchain_subsystem_t::consensus);
    }

    TEST_F(subsystem_traits_test, bigdata_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::bigdata_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::bigdata_subsystem_t::spark)),
                  subsystem::bigdata_subsystem_t::spark);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::bigdata_subsystem_t::etl)),
                  subsystem::bigdata_subsystem_t::etl);
    }

    TEST_F(subsystem_traits_test, devops_subsystem_roundtrip) {
        using traits = subsystem_traits<subsystem::devops_subsystem_t>;
        EXPECT_EQ(traits::from_int(traits::to_int(subsystem::devops_subsystem_t::ci)),
                  subsystem::devops_subsystem_t::ci);
        EXPECT_EQ(traits::from_string(traits::to_string(subsystem::devops_subsystem_t::monitoring)),
                  subsystem::devops_subsystem_t::monitoring);
    }

    TEST_F(subsystem_traits_test, all_subsystem_none_values) {
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_int(0x0000),
                  subsystem::system_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::kernel_subsystem_t>::from_int(0x0100),
                  subsystem::kernel_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::middleware_subsystem_t>::from_int(0x0200),
                  subsystem::middleware_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::application_subsystem_t>::from_int(0x0300),
                  subsystem::application_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::service_subsystem_t>::from_int(0x0400),
                  subsystem::service_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::network_subsystem_t>::from_int(0x0500),
                  subsystem::network_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::storage_subsystem_t>::from_int(0x0600),
                  subsystem::storage_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::database_subsystem_t>::from_int(0x0700),
                  subsystem::database_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::security_subsystem_t>::from_int(0x0800),
                  subsystem::security_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::ai_subsystem_t>::from_int(0x0900), subsystem::ai_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::cloud_subsystem_t>::from_int(0x0A00), subsystem::cloud_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::edge_subsystem_t>::from_int(0x0B00), subsystem::edge_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::iot_subsystem_t>::from_int(0x0C00), subsystem::iot_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::blockchain_subsystem_t>::from_int(0x0D00),
                  subsystem::blockchain_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::bigdata_subsystem_t>::from_int(0x0E00),
                  subsystem::bigdata_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::devops_subsystem_t>::from_int(0x0F00),
                  subsystem::devops_subsystem_t::none);
    }

    TEST_F(subsystem_traits_test, invalid_value_returns_none) {
        EXPECT_EQ(subsystem_traits<subsystem::system_subsystem_t>::from_int(0x9999),
                  subsystem::system_subsystem_t::none);
        EXPECT_EQ(subsystem_traits<subsystem::network_subsystem_t>::from_string("not_exist"),
                  subsystem::network_subsystem_t::none);
    }

    TEST_F(subsystem_traits_test, is_valid_boundary) {
        EXPECT_TRUE(subsystem_traits<subsystem::system_subsystem_t>::is_valid(0x0000));
        EXPECT_TRUE(subsystem_traits<subsystem::system_subsystem_t>::is_valid(0x000D));
        EXPECT_FALSE(subsystem_traits<subsystem::system_subsystem_t>::is_valid(0x000E));
        EXPECT_FALSE(subsystem_traits<subsystem::system_subsystem_t>::is_valid(0xFFFF));
    }

    TEST_F(subsystem_traits_test, to_string_unknown) {
        EXPECT_STREQ(subsystem_traits<subsystem::system_subsystem_t>::to_string(
                         static_cast<subsystem::system_subsystem_t>(0x9999)),
                     "unknown");
    }

}  // namespace error_system::traits
