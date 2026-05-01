#pragma once
#include <cstdint>

/**
 * @file log_module.h
 * @brief 日志管理功能模块定义
 * @details 定义日志管理相关的功能模块分类
 *          范围：0x0900 - 0x09FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 日志管理功能模块分类
     * @details 定义日志管理相关的功能模块，范围 0x0900 - 0x09FF
     */
    enum class log_module_t : uint16_t {
        none = 0x0900,        // 无模块
        appender = 0x0901,    // 日志输出器
        formatter = 0x0902,   // 日志格式化
        collector = 0x0903,   // 日志收集器
        aggregator = 0x0904,  // 日志聚合器
        parser = 0x0905,      // 日志解析器
        filter = 0x0906,      // 日志过滤器
        rotator = 0x0907,     // 日志轮转器
        archiver = 0x0908,    // 日志归档器
        searcher = 0x0909,    // 日志搜索器
        analyzer = 0x090A,    // 日志分析器
        correlator = 0x090B,  // 日志关联器
        shipper = 0x090C,     // 日志转发器
        buffer = 0x090D,      // 日志缓冲器
    };

}  // namespace error_system::module
