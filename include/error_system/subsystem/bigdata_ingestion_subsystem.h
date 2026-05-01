#pragma once
#include <cstdint>

/**
 * @file bigdata_ingestion_subsystem.h
 * @brief 大数据采集子系统分类定义
 * @details 定义大数据采集相关的子系统分类
 *          范围：0x1A00 - 0x1AFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 大数据采集子系统分类
     * @details 定义大数据采集相关的子系统，范围 0x1A00 - 0x1AFF
     */
    enum class bigdata_ingestion_subsystem_t : uint16_t {
        none = 0x1A00,           // 无子系统
        flume = 0x1A01,          // Flume采集
        logstash = 0x1A02,       // Logstash采集
        fluentd = 0x1A03,        // Fluentd采集
        sqoop = 0x1A04,          // Sqoop传输
        debezium = 0x1A05,       // Debezium采集
        kafka_connect = 0x1A06,  // Kafka连接器
        pulsar_io = 0x1A07,      // PulsarIO采集
    };

}  // namespace error_system::subsystem
