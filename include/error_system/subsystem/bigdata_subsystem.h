#pragma once
#include <cstdint>

/**
 * @file bigdata_subsystem.h
 * @brief 大数据层子系统分类定义
 * @details 定义大数据层相关的子系统分类，用于标识错误码所属的大数据层子系统
 *          范围：0x0E00 - 0x0EFF（与大数据层系统域 0x0E 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 大数据层子系统分类
     * @details 定义大数据层相关的子系统，范围 0x0E00 - 0x0EFF
     */
    enum class bigdata_subsystem_t : uint16_t {
        none = 0x0E00,              // 无子系统
        ingestion = 0x0E01,         // 数据采集
        etl = 0x0E02,               // ETL处理
        stream_processing = 0x0E03, // 流处理
        batch_processing = 0x0E04,  // 批处理
        data_warehouse = 0x0E05,    // 数据仓库
        data_lake = 0x0E06,         // 数据湖
        olap = 0x0E07,              // 联机分析处理
        data_mining = 0x0E08,       // 数据挖掘
        machine_learning = 0x0E09,  // 机器学习
        graph_compute = 0x0E0A,     // 图计算
        realtime = 0x0E0B,          // 实时计算
        flink = 0x0E0C,             // Flink计算
        spark = 0x0E0D,             // Spark计算
        hive = 0x0E0E,              // Hive数据仓库
        presto = 0x0E0F,            // Presto查询
        kafka = 0x0E10,             // Kafka数据流
        data_governance = 0x0E11,   // 数据治理
    };

}  // namespace error_system::subsystem
