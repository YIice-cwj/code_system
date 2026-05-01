#pragma once
#include <cstdint>

/**
 * @file kernel_storage_subsystem.h
 * @brief 内核存储子系统分类定义
 * @details 定义内核存储相关的子系统分类
 *          范围：0x0500 - 0x05FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 内核存储子系统分类
     * @details 定义内核存储相关的子系统，范围 0x0500 - 0x05FF
     */
    enum class kernel_storage_subsystem_t : uint16_t {
        none = 0x0500,           // 无子系统
        block_layer = 0x0501,    // 块层管理器
        io_scheduler = 0x0502,   // IO调度器
        elevator = 0x0503,       // 电梯算法器
        scsi_manager = 0x0504,   // SCSI管理器
        nvme_manager = 0x0505,   // NVMe管理器
        raid_engine = 0x0506,    // RAID引擎
        lvm_manager = 0x0507,    // LVM管理器
        crypto_engine = 0x0508,  // 加密引擎
    };

}  // namespace error_system::subsystem
