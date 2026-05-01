#pragma once
#include <cstdint>

/**
 * @file kernel_cpu_subsystem.h
 * @brief 内核CPU子系统分类定义
 * @details 定义内核CPU相关的子系统分类
 *          范围：0x0300 - 0x03FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 内核CPU子系统分类
     * @details 定义内核CPU相关的子系统，范围 0x0300 - 0x03FF
     */
    enum class kernel_cpu_subsystem_t : uint16_t {
        none = 0x0300,              // 无子系统
        scheduler = 0x0301,         // CPU调度器
        load_balancer = 0x0302,     // 负载均衡器
        irq_balancer = 0x0303,      // 中断均衡器
        freq_manager = 0x0304,      // 频率管理器
        thermal_manager = 0x0305,   // 温度管理器
        topology_manager = 0x0306,  // 拓扑管理器
        isolator = 0x0307,          // CPU隔离器
        profiler = 0x0308,          // CPU分析器
    };

}  // namespace error_system::subsystem
