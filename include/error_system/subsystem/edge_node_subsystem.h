#pragma once
#include <cstdint>

/**
 * @file edge_node_subsystem.h
 * @brief 边缘节点子系统分类定义
 * @details 定义边缘节点相关的子系统分类
 *          范围：0x1500 - 0x15FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 边缘节点子系统分类
     * @details 定义边缘节点相关的子系统，范围 0x1500 - 0x15FF
     */
    enum class edge_node_subsystem_t : uint16_t {
        none = 0x1500,               // 无子系统
        container_runtime = 0x1501,  // 容器运行时
        function_runtime = 0x1502,   // 函数运行时
        ai_inference = 0x1503,       // AI推理引擎
        video_analytics = 0x1504,    // 视频分析服务
        data_sync = 0x1505,          // 数据同步服务
        ota_updater = 0x1506,        // 远程升级服务
        monitor_agent = 0x1507,      // 监控代理服务
    };

}  // namespace error_system::subsystem
