#pragma once
#include <cstdint>

/**
 * @file system_subsystem.h
 * @brief 系统层子系统分类定义
 * @details 定义系统层相关的子系统分类，用于标识错误码所属的系统层子系统
 *          范围：0x0000 - 0x00FF（与系统层系统域 0x00 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 系统层子系统分类
     * @details 定义系统层相关的子系统，范围 0x0000 - 0x00FF
     */
    enum class system_subsystem_t : uint16_t {
        none = 0x0000,        // 无子系统
        process = 0x0001,     // 进程管理
        thread = 0x0002,      // 线程管理
        memory = 0x0003,      // 内存管理
        signal = 0x0004,      // 信号处理
        timer = 0x0005,       // 定时器
        ipc = 0x0006,         // 进程间通信
        io = 0x0007,          // IO管理
        filesystem = 0x0008,  // 文件系统
        event = 0x0009,       // 事件系统
        resource = 0x000A,    // 资源管理
        power = 0x000B,       // 电源管理
        log = 0x000C,         // 日志系统
        plugin = 0x000D,      // 插件管理
    };

}  // namespace error_system::subsystem
