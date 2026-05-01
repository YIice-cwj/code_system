#pragma once

/**
 * @file module_traits.h
 * @brief 模块 Traits 统一入口
 * @details 统一包含所有模块 Traits 特化，方便一次性引入
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */

// IWYU pragma: begin_exports
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
// IWYU pragma: end_exports
