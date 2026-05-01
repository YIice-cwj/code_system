#pragma once
#include <cstdint>

/**
 * @file distributed_scheduler_subsystem.h
 * @brief 分布式调度子系统分类定义
 * @details 定义分布式调度相关的子系统分类
 *          范围：0x1F00 - 0x1FFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 分布式调度子系统分类
     * @details 定义分布式调度相关的子系统，范围 0x1F00 - 0x1FFF
     */
    enum class distributed_scheduler_subsystem_t : uint16_t {
        none = 0x1F00,      // 无子系统
        yarn = 0x1F01,      // YARN资源调度
        mesos = 0x1F02,     // Mesos资源调度
        nomad = 0x1F03,     // Nomad工作调度
        airflow = 0x1F04,   // Airflow工作流
        dagster = 0x1F05,   // Dagster工作流
        prefect = 0x1F06,   // Prefect工作流
        temporal = 0x1F07,  // Temporal工作流
        cadence = 0x1F08,   // Cadence工作流
    };

}  // namespace error_system::subsystem
