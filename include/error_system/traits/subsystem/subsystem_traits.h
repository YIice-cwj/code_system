#pragma once

/**
 * @file subsystem_traits.h
 * @brief 子系统 Traits 统一入口
 * @details 统一包含所有子系统 Traits 特化，方便一次性引入
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */

// IWYU pragma: begin_exports
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
// IWYU pragma: end_exports
