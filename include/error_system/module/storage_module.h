#pragma once
#include <cstdint>

/**
 * @file storage_module.h
 * @brief 数据存储功能模块定义
 * @details 定义数据存储相关的功能模块分类
 *          范围：0x0400 - 0x04FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 数据存储功能模块分类
     * @details 定义数据存储相关的功能模块，范围 0x0400 - 0x04FF
     */
    enum class storage_module_t : uint16_t {
        none = 0x0400,               // 无模块
        file_manager = 0x0401,       // 文件管理器
        block_manager = 0x0402,      // 块存储管理
        object_manager = 0x0403,     // 对象存储管理
        volume_manager = 0x0404,     // 卷存储管理
        snapshot_manager = 0x0405,   // 快照管理器
        backup_manager = 0x0406,     // 备份管理器
        replication = 0x0407,        // 数据复制器
        deduplicator = 0x0408,       // 数据去重器
        compressor = 0x0409,         // 数据压缩器
        encryptor = 0x040A,          // 存储加密器
        quota_manager = 0x040B,      // 配额管理器
        lifecycle = 0x040C,          // 生命周期管理
        tiering = 0x040D,            // 分层存储管理
        cache_manager = 0x040E,      // 存储缓存管理
        garbage_collector = 0x040F,  // 垃圾回收器
    };

}  // namespace error_system::module
