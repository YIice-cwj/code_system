#pragma once
#include <cstdint>

/**
 * @file compute_module.h
 * @brief 计算引擎功能模块定义
 * @details 定义计算引擎相关的功能模块分类
 *          范围：0x0700 - 0x07FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 计算引擎功能模块分类
     * @details 定义计算引擎相关的功能模块，范围 0x0700 - 0x07FF
     */
    enum class compute_module_t : uint16_t {
        none = 0x0700,              // 无模块
        executor = 0x0701,          // 任务执行器
        scheduler = 0x0702,         // 任务调度器
        worker_pool = 0x0703,       // 工作线程池
        thread_pool = 0x0704,       // 线程池管理
        coroutine = 0x0705,         // 协程管理器
        async_awaiter = 0x0706,     // 异步等待器
        pipeline = 0x0707,          // 流水线管理
        dag_engine = 0x0708,        // DAG引擎管理
        stream_processor = 0x0709,  // 流处理器
        batch_processor = 0x070A,   // 批处理器
        map_reduce = 0x070B,        // 映射归约器
        filter_engine = 0x070C,     // 过滤引擎
        aggregator = 0x070D,        // 数据聚合器
        sampler = 0x070E,           // 数据采样器
        splitter = 0x070F,          // 数据分割器
        merger = 0x0710,            // 数据合并器
    };

}  // namespace error_system::module
