#pragma once
#include <cstdint>

/**
 * @file database_module.h
 * @brief 数据库功能模块定义
 * @details 定义数据库相关的功能模块分类
 *          范围：0x0700 - 0x07FF（与数据库层系统域 0x07 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 数据库功能模块分类
     * @details 定义数据库相关的功能模块，范围 0x0700 - 0x07FF
     */
    enum class database_module_t : uint16_t {
        none = 0x0700,             // 无模块
        connection = 0x0701,       // 连接管理
        connection_pool = 0x0702,  // 连接池
        transaction = 0x0703,      // 事务管理
        query = 0x0704,            // 查询执行
        statement = 0x0705,        // 语句预处理
        cursor = 0x0706,           // 游标管理
        index = 0x0707,            // 索引管理
        lock = 0x0708,             // 锁管理
        migration = 0x0709,        // 数据迁移
        replication = 0x070A,      // 主从复制
        sharding = 0x070B,         // 分片管理
        backup = 0x070C,           // 备份恢复
        cache = 0x070D,            // 查询缓存
        orm = 0x070E,              // ORM映射
    };

}  // namespace error_system::module
