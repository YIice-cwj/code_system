#pragma once
#include <cstdint>

/**
 * @file bigdata_module.h
 * @brief 大数据层功能模块定义
 * @details 定义大数据层相关的功能模块分类
 *          范围：0x0E00 - 0x0EFF（与大数据层系统域 0x0E 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 大数据层功能模块分类
     * @details 定义大数据层相关的功能模块，范围 0x0E00 - 0x0EFF
     */
    enum class bigdata_module_t : uint16_t {
        none = 0x0E00,                // 无模块
        data_collector = 0x0E01,      // 数据收集器
        extractor = 0x0E02,           // 提取器
        transformer = 0x0E03,         // 转换器
        loader = 0x0E04,              // 加载器
        stream_reader = 0x0E05,       // 流读取器
        stream_writer = 0x0E06,       // 流写入器
        batch_scheduler = 0x0E07,     // 批调度器
        query_engine = 0x0E08,        // 查询引擎
        cube_builder = 0x0E09,        // 立方体构建器
        model_trainer = 0x0E0A,       // 模型训练器
        graph_loader = 0x0E0B,        // 图加载器
        window_operator = 0x0E0C,     // 窗口算子
        checkpoint_manager = 0x0E0D,  // 检查点管理器
        lineage_tracker = 0x0E0E,     // 血缘追踪器
        quality_checker = 0x0E0F,     // 质量检查器
        catalog_manager = 0x0E10,     // 目录管理器
    };

}  // namespace error_system::module
