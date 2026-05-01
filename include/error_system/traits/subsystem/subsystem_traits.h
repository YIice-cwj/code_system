#pragma once

/**
 * @file subsystem_traits.h
 * @brief Subsystem Traits 统一入口
 */

// IWYU pragma: begin_exports
#include "error_system/traits/subsystem/ai_llm_subsystem_traits.h"
#include "error_system/traits/subsystem/ai_vision_subsystem_traits.h"
#include "error_system/traits/subsystem/application_mobile_subsystem_traits.h"
#include "error_system/traits/subsystem/application_web_subsystem_traits.h"
#include "error_system/traits/subsystem/bigdata_compute_subsystem_traits.h"
#include "error_system/traits/subsystem/bigdata_ingestion_subsystem_traits.h"
#include "error_system/traits/subsystem/blockchain_consensus_subsystem_traits.h"
#include "error_system/traits/subsystem/blockchain_contract_subsystem_traits.h"
#include "error_system/traits/subsystem/cloud_compute_subsystem_traits.h"
#include "error_system/traits/subsystem/cloud_storage_subsystem_traits.h"
#include "error_system/traits/subsystem/database_nosql_subsystem_traits.h"
#include "error_system/traits/subsystem/database_sql_subsystem_traits.h"
#include "error_system/traits/subsystem/devops_cd_subsystem_traits.h"
#include "error_system/traits/subsystem/devops_ci_subsystem_traits.h"
#include "error_system/traits/subsystem/distributed_coordination_subsystem_traits.h"
#include "error_system/traits/subsystem/distributed_scheduler_subsystem_traits.h"
#include "error_system/traits/subsystem/edge_gateway_subsystem_traits.h"
#include "error_system/traits/subsystem/edge_node_subsystem_traits.h"
#include "error_system/traits/subsystem/iot_device_subsystem_traits.h"
#include "error_system/traits/subsystem/iot_network_subsystem_traits.h"
#include "error_system/traits/subsystem/kernel_cpu_subsystem_traits.h"
#include "error_system/traits/subsystem/kernel_network_subsystem_traits.h"
#include "error_system/traits/subsystem/kernel_storage_subsystem_traits.h"
#include "error_system/traits/subsystem/middleware_mq_subsystem_traits.h"
#include "error_system/traits/subsystem/middleware_rpc_subsystem_traits.h"
#include "error_system/traits/subsystem/monitoring_alerting_subsystem_traits.h"
#include "error_system/traits/subsystem/monitoring_observability_subsystem_traits.h"
#include "error_system/traits/subsystem/security_auth_subsystem_traits.h"
#include "error_system/traits/subsystem/security_crypto_subsystem_traits.h"
#include "error_system/traits/subsystem/service_microservice_subsystem_traits.h"
#include "error_system/traits/subsystem/service_serverless_subsystem_traits.h"
#include "error_system/traits/subsystem/system_io_subsystem_traits.h"
#include "error_system/traits/subsystem/system_memory_subsystem_traits.h"
#include "error_system/traits/subsystem/system_process_subsystem_traits.h"
// IWYU pragma: end_exports