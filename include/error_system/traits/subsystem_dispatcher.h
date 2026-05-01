#pragma once
#include <cstdint>
#include "error_system/subsystem/subsystem.h"
#include "error_system/traits/subsystem/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief 根据子系统枚举值解析对应的子系统名称
     * @param subsystem_value 子系统枚举值
     * @return const char* 子系统名称，若无匹配则返回 "none"
     */
    static constexpr const char* resolve_subsystem(uint16_t subsystem_value) noexcept {
        if (traits::subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::ai_llm_subsystem_t>::to_string(static_cast<subsystem::ai_llm_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::ai_vision_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::ai_vision_subsystem_t>::to_string(static_cast<subsystem::ai_vision_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::application_mobile_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::application_mobile_subsystem_t>::to_string(static_cast<subsystem::application_mobile_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::application_web_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::application_web_subsystem_t>::to_string(static_cast<subsystem::application_web_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::bigdata_compute_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::bigdata_compute_subsystem_t>::to_string(static_cast<subsystem::bigdata_compute_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::bigdata_ingestion_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::bigdata_ingestion_subsystem_t>::to_string(static_cast<subsystem::bigdata_ingestion_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::blockchain_consensus_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::blockchain_consensus_subsystem_t>::to_string(static_cast<subsystem::blockchain_consensus_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::blockchain_contract_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::blockchain_contract_subsystem_t>::to_string(static_cast<subsystem::blockchain_contract_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::cloud_compute_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::cloud_compute_subsystem_t>::to_string(static_cast<subsystem::cloud_compute_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::cloud_storage_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::cloud_storage_subsystem_t>::to_string(static_cast<subsystem::cloud_storage_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::database_nosql_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::database_nosql_subsystem_t>::to_string(static_cast<subsystem::database_nosql_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::database_sql_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::database_sql_subsystem_t>::to_string(static_cast<subsystem::database_sql_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::devops_cd_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::devops_cd_subsystem_t>::to_string(static_cast<subsystem::devops_cd_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::devops_ci_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::devops_ci_subsystem_t>::to_string(static_cast<subsystem::devops_ci_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::distributed_coordination_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::distributed_coordination_subsystem_t>::to_string(static_cast<subsystem::distributed_coordination_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::distributed_scheduler_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::distributed_scheduler_subsystem_t>::to_string(static_cast<subsystem::distributed_scheduler_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::edge_gateway_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::edge_gateway_subsystem_t>::to_string(static_cast<subsystem::edge_gateway_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::edge_node_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::edge_node_subsystem_t>::to_string(static_cast<subsystem::edge_node_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::iot_device_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::iot_device_subsystem_t>::to_string(static_cast<subsystem::iot_device_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::iot_network_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::iot_network_subsystem_t>::to_string(static_cast<subsystem::iot_network_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::kernel_cpu_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::kernel_cpu_subsystem_t>::to_string(static_cast<subsystem::kernel_cpu_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::kernel_network_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::kernel_network_subsystem_t>::to_string(static_cast<subsystem::kernel_network_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::kernel_storage_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::kernel_storage_subsystem_t>::to_string(static_cast<subsystem::kernel_storage_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::middleware_mq_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::middleware_mq_subsystem_t>::to_string(static_cast<subsystem::middleware_mq_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::middleware_rpc_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::middleware_rpc_subsystem_t>::to_string(static_cast<subsystem::middleware_rpc_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::monitoring_alerting_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::monitoring_alerting_subsystem_t>::to_string(static_cast<subsystem::monitoring_alerting_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::monitoring_observability_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::monitoring_observability_subsystem_t>::to_string(static_cast<subsystem::monitoring_observability_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::security_auth_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::security_auth_subsystem_t>::to_string(static_cast<subsystem::security_auth_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::security_crypto_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::security_crypto_subsystem_t>::to_string(static_cast<subsystem::security_crypto_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::service_microservice_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::service_microservice_subsystem_t>::to_string(static_cast<subsystem::service_microservice_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::service_serverless_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::service_serverless_subsystem_t>::to_string(static_cast<subsystem::service_serverless_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::system_io_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::system_io_subsystem_t>::to_string(static_cast<subsystem::system_io_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::system_memory_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::system_memory_subsystem_t>::to_string(static_cast<subsystem::system_memory_subsystem_t>(subsystem_value));
        if (traits::subsystem_traits<subsystem::system_process_subsystem_t>::is_valid(subsystem_value))
            return traits::subsystem_traits<subsystem::system_process_subsystem_t>::to_string(static_cast<subsystem::system_process_subsystem_t>(subsystem_value));
        return "none";
    }

    /**
     * @brief 根据子系统类型和名称解析枚举值
     * @param type 子系统类型字符串 (如 "ai_llm", "kernel_cpu")
     * @param name 子系统内枚举名称
     * @return uint16_t 子系统枚举值，若无匹配则返回 0
     */
    static constexpr uint16_t subsystem_from_string(const char* type, const char* name) noexcept {
        switch (utils::string_utils_t::hash(type)) {
            case utils::string_utils_t::hash("ai_llm"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::ai_llm_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("ai_vision"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::ai_vision_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("application_mobile"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::application_mobile_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("application_web"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::application_web_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("bigdata_compute"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::bigdata_compute_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("bigdata_ingestion"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::bigdata_ingestion_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("blockchain_consensus"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::blockchain_consensus_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("blockchain_contract"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::blockchain_contract_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("cloud_compute"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::cloud_compute_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("cloud_storage"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::cloud_storage_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("database_nosql"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::database_nosql_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("database_sql"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::database_sql_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("devops_cd"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::devops_cd_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("devops_ci"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::devops_ci_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("distributed_coordination"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::distributed_coordination_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("distributed_scheduler"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::distributed_scheduler_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("edge_gateway"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::edge_gateway_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("edge_node"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::edge_node_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("iot_device"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::iot_device_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("iot_network"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::iot_network_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("kernel_cpu"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::kernel_cpu_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("kernel_network"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::kernel_network_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("kernel_storage"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::kernel_storage_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("middleware_mq"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::middleware_mq_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("middleware_rpc"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::middleware_rpc_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("monitoring_alerting"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::monitoring_alerting_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("monitoring_observability"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::monitoring_observability_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("security_auth"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::security_auth_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("security_crypto"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::security_crypto_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("service_microservice"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::service_microservice_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("service_serverless"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::service_serverless_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("system_io"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::system_io_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("system_memory"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::system_memory_subsystem_t>::from_string(name));
            case utils::string_utils_t::hash("system_process"):
                return static_cast<uint16_t>(traits::subsystem_traits<subsystem::system_process_subsystem_t>::from_string(name));
            default:
                return 0;
        }
    }

}  // namespace error_system::traits
