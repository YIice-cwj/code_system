#pragma once
#include <cstdint>

/**
 * @file kernel_subsystem.h
 * @brief 内核层子系统分类定义
 * @details 定义内核层相关的子系统分类，用于标识错误码所属的内核层子系统
 *          范围：0x0100 - 0x01FF（与内核层系统域 0x01 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 内核层子系统分类
     * @details 定义内核层相关的子系统，范围 0x0100 - 0x01FF
     */
    enum class kernel_subsystem_t : uint16_t {
        none = 0x0100,       // 无子系统
        scheduler = 0x0101,  // 任务调度器
        interrupt = 0x0102,  // 中断管理
        driver = 0x0103,     // 设备驱动
        vfs = 0x0104,        // 虚拟文件系统
        netstack = 0x0105,   // 网络协议栈
        syscall = 0x0106,    // 系统调用
        module = 0x0107,     // 内核模块
    };

}  // namespace error_system::subsystem
