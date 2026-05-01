#pragma once
#include <cstdint>

/**
 * @file system_process_subsystem.h
 * @brief 系统进程子系统分类定义
 * @details 定义系统进程相关的子系统分类
 *          范围：0x0000 - 0x00FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 系统进程子系统分类
     * @details 定义系统进程相关的子系统，范围 0x0000 - 0x00FF
     */
    enum class system_process_subsystem_t : uint16_t {
        none = 0x0000,               // 无子系统
        scheduler = 0x0001,          // 进程调度器
        fork_manager = 0x0002,       // 进程创建器
        signal_handler = 0x0003,     // 信号处理器
        zombie_reaper = 0x0004,      // 僵尸进程回收
        priority_manager = 0x0005,   // 优先级管理
        cgroup_manager = 0x0006,     // 控制组管理
        namespace_manager = 0x0007,  // 命名空间管理
        ipc_manager = 0x0008,        // 进程通信管理
        daemon_manager = 0x0009,     // 守护进程管理
    };

}  // namespace error_system::subsystem
