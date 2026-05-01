#pragma once
#include <cstdint>

/**
 * @file database_module.h
 * @brief 数据库访问功能模块定义
 * @details 定义数据库访问相关的功能模块分类
 *          范围：0x0200 - 0x02FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 数据库访问功能模块分类
     * @details 定义数据库访问相关的功能模块，范围 0x0200 - 0x02FF
     */
    enum class database_module_t : uint16_t {
        none = 0x0200,               // 无模块
        connector = 0x0201,          // 数据库连接器
        connection_pool = 0x0202,    // 连接池管理
        transaction = 0x0203,        // 事务管理器
        query_executor = 0x0204,     // 查询执行器
        statement = 0x0205,          // 语句预处理器
        cursor = 0x0206,             // 游标管理器
        index_manager = 0x0207,      // 索引管理器
        lock_manager = 0x0208,       // 锁管理器
        migration = 0x0209,          // 数据迁移器
        replicator = 0x020A,         // 主从复制器
        shard_manager = 0x020B,      // 分片管理器
        backup_restore = 0x020C,     // 备份恢复器
        cache_manager = 0x020D,      // 查询缓存器
        orm_mapper = 0x020E,         // ORM映射器
        schema_manager = 0x020F,     // 模式管理器
        trigger_manager = 0x0210,    // 触发器管理
        view_manager = 0x0211,       // 视图管理器
        procedure = 0x0212,          // 存储过程器
        function_manager = 0x0213,   // 函数管理器
        partition_manager = 0x0214,  // 分区管理器
        vacuum = 0x0215,             // 空间回收器
        wal_manager = 0x0216,        // 预写日志管理
        cluster_manager = 0x0217,    // 集群管理器
        monitor = 0x0218,            // 监控管理器
    };

}  // namespace error_system::module
