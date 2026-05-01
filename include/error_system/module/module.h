#pragma once

/**
 * @file module.h
 * @brief 功能模块统一入口
 * @details 统一包含所有功能模块定义，方便用户一次性引入所有模块
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */

// IWYU pragma: begin_exports
#include "error_system/module/ai_module.h"
#include "error_system/module/cache_module.h"
#include "error_system/module/common_module.h"
#include "error_system/module/compute_module.h"
#include "error_system/module/config_module.h"
#include "error_system/module/database_module.h"
#include "error_system/module/log_module.h"
#include "error_system/module/message_module.h"
#include "error_system/module/monitor_module.h"
#include "error_system/module/network_module.h"
#include "error_system/module/security_module.h"
#include "error_system/module/storage_module.h"
// IWYU pragma: end_exports
