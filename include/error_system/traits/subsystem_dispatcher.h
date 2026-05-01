#pragma once

/**
 * @file subsystem_dispatcher.h
 * @brief Subsystem dispatcher for metadata resolution
 * @details 自动生成的 subsystem 分发器，用于根据数值范围或类型字符串解析元数据
 * @author antigravity
 * @version 1.0.0
 * @copyright Copyright (c) 2026
 */

#include <cstdint>
#include "error_system/subsystem/subsystem.h"
#include "error_system/traits/subsystem/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {

    /**
     * @brief 根据数值解析 subsystem 分类类型名称 (用于 I18N 路径)
     * @param value subsystem 的整数值
     * @return const char* 对应的分类名称字符串 (如 "ai", "cache")，若无匹配返回 "none"
     */
    static constexpr const char* resolve_subsystem_type(uint16_t value) noexcept {
        if (traits::subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(value))
            return "ai_llm";
        if (traits::subsystem_traits<subsystem::ai_vision_subsystem_t>::is_valid(value))
            return "ai_vision";
        if (traits::subsystem_traits<subsystem::application_mobile_subsystem_t>::is_valid(value))
            return "application_mobile";
        if (traits::subsystem_traits<subsystem::application_web_subsystem_t>::is_valid(value))
            return "application_web";
        if (traits::subsystem_traits<subsystem::bigdata_compute_subsystem_t>::is_valid(value))
            return "bigdata_compute";
        if (traits::subsystem_traits<subsystem::bigdata_ingestion_subsystem_t>::is_valid(value))
            return "bigdata_ingestion";
        if (traits::subsystem_traits<subsystem::blockchain_consensus_subsystem_t>::is_valid(value))
            return "blockchain_consensus";
        if (traits::subsystem_traits<subsystem::blockchain_contract_subsystem_t>::is_valid(value))
            return "blockchain_contract";
        if (traits::subsystem_traits<subsystem::cloud_compute_subsystem_t>::is_valid(value))
            return "cloud_compute";
        if (traits::subsystem_traits<subsystem::cloud_storage_subsystem_t>::is_valid(value))
            return "cloud_storage";
        if (traits::subsystem_traits<subsystem::database_nosql_subsystem_t>::is_valid(value))
            return "database_nosql";
        if (traits::subsystem_traits<subsystem::database_sql_subsystem_t>::is_valid(value))
            return "database_sql";
        if (traits::subsystem_traits<subsystem::devops_cd_subsystem_t>::is_valid(value))
            return "devops_cd";
        if (traits::subsystem_traits<subsystem::devops_ci_subsystem_t>::is_valid(value))
            return "devops_ci";
        if (traits::subsystem_traits<subsystem::distributed_coordination_subsystem_t>::is_valid(value))
            return "distributed_coordination";
        if (traits::subsystem_traits<subsystem::distributed_scheduler_subsystem_t>::is_valid(value))
            return "distributed_scheduler";
        if (traits::subsystem_traits<subsystem::edge_gateway_subsystem_t>::is_valid(value))
            return "edge_gateway";
        if (traits::subsystem_traits<subsystem::edge_node_subsystem_t>::is_valid(value))
            return "edge_node";
        if (traits::subsystem_traits<subsystem::iot_device_subsystem_t>::is_valid(value))
            return "iot_device";
        if (traits::subsystem_traits<subsystem::iot_network_subsystem_t>::is_valid(value))
            return "iot_network";
        if (traits::subsystem_traits<subsystem::kernel_cpu_subsystem_t>::is_valid(value))
            return "kernel_cpu";
        if (traits::subsystem_traits<subsystem::kernel_network_subsystem_t>::is_valid(value))
            return "kernel_network";
        if (traits::subsystem_traits<subsystem::kernel_storage_subsystem_t>::is_valid(value))
            return "kernel_storage";
        if (traits::subsystem_traits<subsystem::middleware_mq_subsystem_t>::is_valid(value))
            return "middleware_mq";
        if (traits::subsystem_traits<subsystem::middleware_rpc_subsystem_t>::is_valid(value))
            return "middleware_rpc";
        if (traits::subsystem_traits<subsystem::monitoring_alerting_subsystem_t>::is_valid(value))
            return "monitoring_alerting";
        if (traits::subsystem_traits<subsystem::monitoring_observability_subsystem_t>::is_valid(value))
            return "monitoring_observability";
        if (traits::subsystem_traits<subsystem::security_auth_subsystem_t>::is_valid(value))
            return "security_auth";
        if (traits::subsystem_traits<subsystem::security_crypto_subsystem_t>::is_valid(value))
            return "security_crypto";
        if (traits::subsystem_traits<subsystem::service_microservice_subsystem_t>::is_valid(value))
            return "service_microservice";
        if (traits::subsystem_traits<subsystem::service_serverless_subsystem_t>::is_valid(value))
            return "service_serverless";
        if (traits::subsystem_traits<subsystem::system_io_subsystem_t>::is_valid(value))
            return "system_io";
        if (traits::subsystem_traits<subsystem::system_memory_subsystem_t>::is_valid(value))
            return "system_memory";
        if (traits::subsystem_traits<subsystem::system_process_subsystem_t>::is_valid(value))
            return "system_process";
        return "none";
    }

    /**
     * @brief 根据数值解析 subsystem 名称
     * @param value subsystem 的整数值
     * @return const char* 对应的名称字符串，若无匹配返回 "none"
     */
    static constexpr const char* resolve_subsystem(uint16_t value) noexcept {
        if (traits::subsystem_traits<subsystem::ai_llm_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::ai_llm_subsystem_t>::to_string(static_cast<subsystem::ai_llm_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::ai_vision_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::ai_vision_subsystem_t>::to_string(static_cast<subsystem::ai_vision_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::application_mobile_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::application_mobile_subsystem_t>::to_string(static_cast<subsystem::application_mobile_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::application_web_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::application_web_subsystem_t>::to_string(static_cast<subsystem::application_web_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::bigdata_compute_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::bigdata_compute_subsystem_t>::to_string(static_cast<subsystem::bigdata_compute_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::bigdata_ingestion_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::bigdata_ingestion_subsystem_t>::to_string(static_cast<subsystem::bigdata_ingestion_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::blockchain_consensus_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::blockchain_consensus_subsystem_t>::to_string(static_cast<subsystem::blockchain_consensus_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::blockchain_contract_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::blockchain_contract_subsystem_t>::to_string(static_cast<subsystem::blockchain_contract_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::cloud_compute_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::cloud_compute_subsystem_t>::to_string(static_cast<subsystem::cloud_compute_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::cloud_storage_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::cloud_storage_subsystem_t>::to_string(static_cast<subsystem::cloud_storage_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::database_nosql_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::database_nosql_subsystem_t>::to_string(static_cast<subsystem::database_nosql_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::database_sql_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::database_sql_subsystem_t>::to_string(static_cast<subsystem::database_sql_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::devops_cd_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::devops_cd_subsystem_t>::to_string(static_cast<subsystem::devops_cd_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::devops_ci_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::devops_ci_subsystem_t>::to_string(static_cast<subsystem::devops_ci_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::distributed_coordination_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::distributed_coordination_subsystem_t>::to_string(static_cast<subsystem::distributed_coordination_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::distributed_scheduler_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::distributed_scheduler_subsystem_t>::to_string(static_cast<subsystem::distributed_scheduler_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::edge_gateway_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::edge_gateway_subsystem_t>::to_string(static_cast<subsystem::edge_gateway_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::edge_node_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::edge_node_subsystem_t>::to_string(static_cast<subsystem::edge_node_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::iot_device_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::iot_device_subsystem_t>::to_string(static_cast<subsystem::iot_device_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::iot_network_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::iot_network_subsystem_t>::to_string(static_cast<subsystem::iot_network_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::kernel_cpu_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::kernel_cpu_subsystem_t>::to_string(static_cast<subsystem::kernel_cpu_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::kernel_network_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::kernel_network_subsystem_t>::to_string(static_cast<subsystem::kernel_network_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::kernel_storage_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::kernel_storage_subsystem_t>::to_string(static_cast<subsystem::kernel_storage_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::middleware_mq_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::middleware_mq_subsystem_t>::to_string(static_cast<subsystem::middleware_mq_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::middleware_rpc_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::middleware_rpc_subsystem_t>::to_string(static_cast<subsystem::middleware_rpc_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::monitoring_alerting_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::monitoring_alerting_subsystem_t>::to_string(static_cast<subsystem::monitoring_alerting_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::monitoring_observability_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::monitoring_observability_subsystem_t>::to_string(static_cast<subsystem::monitoring_observability_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::security_auth_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::security_auth_subsystem_t>::to_string(static_cast<subsystem::security_auth_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::security_crypto_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::security_crypto_subsystem_t>::to_string(static_cast<subsystem::security_crypto_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::service_microservice_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::service_microservice_subsystem_t>::to_string(static_cast<subsystem::service_microservice_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::service_serverless_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::service_serverless_subsystem_t>::to_string(static_cast<subsystem::service_serverless_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::system_io_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::system_io_subsystem_t>::to_string(static_cast<subsystem::system_io_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::system_memory_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::system_memory_subsystem_t>::to_string(static_cast<subsystem::system_memory_subsystem_t>(value));
        if (traits::subsystem_traits<subsystem::system_process_subsystem_t>::is_valid(value))
            return traits::subsystem_traits<subsystem::system_process_subsystem_t>::to_string(static_cast<subsystem::system_process_subsystem_t>(value));
        return "none";
    }

    /**
     * @brief 根据类型和名称解析 subsystem 枚举值
     * @param type subsystem 的分类名称
     * @param name subsystem 的具体名称
     * @return uint16_t 对应的枚举整数值，若无匹配返回 0
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
            default: return 0;
        }
    }

}  // namespace error_system::traits