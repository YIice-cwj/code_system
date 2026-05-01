#pragma once
#include <cstdint>

/**
 * @file service_serverless_subsystem.h
 * @brief 服务无服务器子系统分类定义
 * @details 定义服务无服务器相关的子系统分类
 *          范围：0x0B00 - 0x0BFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 服务无服务器子系统分类
     * @details 定义服务无服务器相关的子系统，范围 0x0B00 - 0x0BFF
     */
    enum class service_serverless_subsystem_t : uint16_t {
        none = 0x0B00,                // 无子系统
        function_runtime = 0x0B01,    // 函数运行时
        event_trigger = 0x0B02,       // 事件触发器
        step_function = 0x0B03,       // 步骤函数
        schedule_timer = 0x0B04,      // 定时调度器
        cold_start = 0x0B05,          // 冷启动管理
        warm_pool = 0x0B06,           // 预热池管理
        scaling_controller = 0x0B07,  // 扩缩控制器
    };

}  // namespace error_system::subsystem
