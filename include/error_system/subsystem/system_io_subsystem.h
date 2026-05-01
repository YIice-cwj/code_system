#pragma once
#include <cstdint>

/**
 * @file system_io_subsystem.h
 * @brief 系统IO子系统分类定义
 * @details 定义系统IO相关的子系统分类
 *          范围：0x0200 - 0x02FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 系统IO子系统分类
     * @details 定义系统IO相关的子系统，范围 0x0200 - 0x02FF
     */
    enum class system_io_subsystem_t : uint16_t {
        none = 0x0200,               // 无子系统
        vfs = 0x0201,                // 虚拟文件系统
        block_io = 0x0202,           // 块设备IO
        char_io = 0x0203,            // 字符设备IO
        async_io = 0x0204,           // 异步IO管理
        dma_engine = 0x0205,         // DMA引擎管理
        interrupt_handler = 0x0206,  // 中断处理器
        device_mapper = 0x0207,      // 设备映射器
        raid_manager = 0x0208,       // RAID管理器
        fs_manager = 0x0209,         // 文件系统管理
    };

}  // namespace error_system::subsystem
