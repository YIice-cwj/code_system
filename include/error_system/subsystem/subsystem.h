#pragma once

/**
 * @file subsystem.h
 * @brief 子系统统一入口
 * @details 统一包含所有子系统定义，方便用户一次性引入所有子系统
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */

// IWYU pragma: begin_exports
#include "error_system/subsystem/ai_llm_subsystem.h"
#include "error_system/subsystem/ai_vision_subsystem.h"
#include "error_system/subsystem/application_mobile_subsystem.h"
#include "error_system/subsystem/application_web_subsystem.h"
#include "error_system/subsystem/bigdata_compute_subsystem.h"
#include "error_system/subsystem/bigdata_ingestion_subsystem.h"
#include "error_system/subsystem/blockchain_consensus_subsystem.h"
#include "error_system/subsystem/blockchain_contract_subsystem.h"
#include "error_system/subsystem/cloud_compute_subsystem.h"
#include "error_system/subsystem/cloud_storage_subsystem.h"
#include "error_system/subsystem/database_nosql_subsystem.h"
#include "error_system/subsystem/database_sql_subsystem.h"
#include "error_system/subsystem/devops_cd_subsystem.h"
#include "error_system/subsystem/devops_ci_subsystem.h"
#include "error_system/subsystem/distributed_coordination_subsystem.h"
#include "error_system/subsystem/distributed_scheduler_subsystem.h"
#include "error_system/subsystem/edge_gateway_subsystem.h"
#include "error_system/subsystem/edge_node_subsystem.h"
#include "error_system/subsystem/iot_device_subsystem.h"
#include "error_system/subsystem/iot_network_subsystem.h"
#include "error_system/subsystem/kernel_cpu_subsystem.h"
#include "error_system/subsystem/kernel_network_subsystem.h"
#include "error_system/subsystem/kernel_storage_subsystem.h"
#include "error_system/subsystem/middleware_mq_subsystem.h"
#include "error_system/subsystem/middleware_rpc_subsystem.h"
#include "error_system/subsystem/monitoring_alerting_subsystem.h"
#include "error_system/subsystem/monitoring_observability_subsystem.h"
#include "error_system/subsystem/security_auth_subsystem.h"
#include "error_system/subsystem/security_crypto_subsystem.h"
#include "error_system/subsystem/service_microservice_subsystem.h"
#include "error_system/subsystem/service_serverless_subsystem.h"
#include "error_system/subsystem/system_io_subsystem.h"
#include "error_system/subsystem/system_memory_subsystem.h"
#include "error_system/subsystem/system_process_subsystem.h"
// IWYU pragma: end_exports
