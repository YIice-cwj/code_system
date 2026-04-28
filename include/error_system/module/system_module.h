#pragma once
#include <cstdint>

/**
 * @file system_module.h
 * @brief 系统层功能模块定义
 * @details 定义系统层相关的功能模块分类
 *          范围：0x0000 - 0x00FF（与系统层系统域 0x00 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 系统层功能模块分类
     * @details 定义系统层相关的功能模块，范围 0x0000 - 0x00FF
     */
    enum class system_module_t : uint16_t {
        none = 0x0000,       // 无模块
        process = 0x0001,    // 进程管理
        thread = 0x0002,     // 线程管理
        memory = 0x0003,     // 内存管理
        signal = 0x0004,     // 信号处理
        timer = 0x0005,      // 定时器
        ipc = 0x0006,        // 进程间通信
        io = 0x0007,         // IO管理
        event = 0x0008,      // 事件管理
        semaphore = 0x0009,  // 信号量
        mutex = 0x000A,      // 互斥锁
        rwlock = 0x000B,     // 读写锁
        condition = 0x000C,  // 条件变量
    };

}  // namespace error_system::module
