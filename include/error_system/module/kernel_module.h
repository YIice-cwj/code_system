#pragma once
#include <cstdint>

/**
 * @file kernel_module.h
 * @brief 内核层功能模块定义
 * @details 定义内核层相关的功能模块分类
 *          范围：0x0100 - 0x01FF（与内核层系统域 0x01 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 内核层功能模块分类
     * @details 定义内核层相关的功能模块，范围 0x0100 - 0x01FF
     */
    enum class kernel_module_t : uint16_t {
        none = 0x0100,           // 无模块
        scheduler = 0x0101,      // 任务调度
        interrupt = 0x0102,      // 中断管理
        driver = 0x0103,         // 设备驱动
        vfs = 0x0104,            // 虚拟文件系统
        netstack = 0x0105,       // 网络协议栈
        syscall = 0x0106,        // 系统调用
        module_loader = 0x0107,  // 模块加载器
        page_table = 0x0108,     // 页表管理
        dma = 0x0109,            // DMA管理
        buffer = 0x010A,         // 缓冲区管理
        cache = 0x010B,          // 缓存管理
    };

}  // namespace error_system::module
