#pragma once
#include <cstdint>

/**
 * @file cloud_storage_subsystem.h
 * @brief 云计算存储子系统分类定义
 * @details 定义云计算存储相关的子系统分类
 *          范围：0x1300 - 0x13FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 云计算存储子系统分类
     * @details 定义云计算存储相关的子系统，范围 0x1300 - 0x13FF
     */
    enum class cloud_storage_subsystem_t : uint16_t {
        none = 0x1300,              // 无子系统
        object_storage = 0x1301,    // 对象存储服务
        block_storage = 0x1302,     // 块存储服务
        file_storage = 0x1303,      // 文件存储服务
        archive_storage = 0x1304,   // 归档存储服务
        cdn = 0x1305,               // 内容分发网络
        backup_service = 0x1306,    // 云备份服务
        snapshot_service = 0x1307,  // 云快照服务
        replication = 0x1308,       // 跨区域复制
    };

}  // namespace error_system::subsystem
