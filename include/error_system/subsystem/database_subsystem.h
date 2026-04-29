#pragma once
#include <cstdint>

/**
 * @file database_subsystem.h
 * @brief 数据库子系统分类定义
 * @details 定义数据库相关的子系统分类，用于标识错误码所属的数据库子系统
 *          范围：0x0700 - 0x07FF（与数据库层系统域 0x07 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 数据库子系统分类
     * @details 定义数据库相关的子系统，范围 0x0700 - 0x07FF
     */
    enum class database_subsystem_t : uint16_t {
        none = 0x0700,           // 无子系统
        mysql = 0x0701,          // MySQL数据库
        redis = 0x0702,          // Redis缓存
        mongodb = 0x0703,        // MongoDB数据库
        postgresql = 0x0704,     // PostgreSQL数据库
        sqlite = 0x0705,         // SQLite数据库
        elasticsearch = 0x0706,  // Elasticsearch搜索引擎
        clickhouse = 0x0707,     // ClickHouse列式数据库
        tidb = 0x0708,           // TiDB分布式数据库
        oracle = 0x0709,         // Oracle数据库
        sqlserver = 0x070A,      // SQL Server数据库
        mariadb = 0x070B,        // MariaDB数据库
        neo4j = 0x070C,          // Neo4j图数据库
        influxdb = 0x070D,       // InfluxDB时序数据库
        etcd = 0x070E,           // Etcd键值存储
    };

}  // namespace error_system::subsystem
