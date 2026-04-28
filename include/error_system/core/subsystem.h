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

#include "error_system/subsystem/application_subsystem.h"
#include "error_system/subsystem/database_subsystem.h"
#include "error_system/subsystem/kernel_subsystem.h"
#include "error_system/subsystem/middleware_subsystem.h"
#include "error_system/subsystem/network_subsystem.h"
#include "error_system/subsystem/service_subsystem.h"
#include "error_system/subsystem/storage_subsystem.h"
#include "error_system/subsystem/system_subsystem.h"
