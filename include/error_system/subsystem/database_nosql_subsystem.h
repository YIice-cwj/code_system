#pragma once
#include <cstdint>

/**
 * @file database_nosql_subsystem.h
 * @brief 数据库NoSQL子系统分类定义
 * @details 定义数据库NoSQL相关的子系统分类
 *          范围：0x0D00 - 0x0DFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 数据库NoSQL子系统分类
     * @details 定义数据库NoSQL相关的子系统，范围 0x0D00 - 0x0DFF
     */
    enum class database_nosql_subsystem_t : uint16_t {
        none = 0x0D00,           // 无子系统
        mongodb = 0x0D01,        // MongoDB数据库
        redis = 0x0D02,          // Redis缓存
        cassandra = 0x0D03,      // Cassandra数据库
        elasticsearch = 0x0D04,  // Elasticsearch搜索
        clickhouse = 0x0D05,     // ClickHouse列式
        influxdb = 0x0D06,       // InfluxDB时序
        neo4j = 0x0D07,          // Neo4j图数据库
        etcd = 0x0D08,           // Etcd键值存储
        tidb = 0x0D09,           // TiDB分布式库
    };

}  // namespace error_system::subsystem
