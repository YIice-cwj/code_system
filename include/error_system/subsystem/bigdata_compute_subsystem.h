#pragma once
#include <cstdint>

/**
 * @file bigdata_compute_subsystem.h
 * @brief 大数据计算子系统分类定义
 * @details 定义大数据计算相关的子系统分类
 *          范围：0x1B00 - 0x1BFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 大数据计算子系统分类
     * @details 定义大数据计算相关的子系统，范围 0x1B00 - 0x1BFF
     */
    enum class bigdata_compute_subsystem_t : uint16_t {
        none = 0x1B00,              // 无子系统
        spark_engine = 0x1B01,      // Spark计算引擎
        flink_engine = 0x1B02,      // Flink计算引擎
        hive_engine = 0x1B03,       // Hive查询引擎
        presto_engine = 0x1B04,     // Presto查询引擎
        druid_engine = 0x1B05,      // Druid分析引擎
        trino_engine = 0x1B06,      // Trino查询引擎
        mapreduce_engine = 0x1B07,  // MapReduce引擎
        tez_engine = 0x1B08,        // Tez计算引擎
    };

}  // namespace error_system::subsystem
