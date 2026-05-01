#pragma once
#include <cstdint>

/**
 * @file monitor_module.h
 * @brief 监控告警功能模块定义
 * @details 定义监控告警相关的功能模块分类
 *          范围：0x0A00 - 0x0AFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 监控告警功能模块分类
     * @details 定义监控告警相关的功能模块，范围 0x0A00 - 0x0AFF
     */
    enum class monitor_module_t : uint16_t {
        none = 0x0A00,               // 无模块
        metric_collector = 0x0A01,   // 指标收集器
        metric_aggregator = 0x0A02,  // 指标聚合器
        alert_manager = 0x0A03,      // 告警管理器
        notifier = 0x0A04,           // 通知发送器
        dashboard = 0x0A05,          // 仪表盘管理
        probe = 0x0A06,              // 探测管理器
        sampler = 0x0A07,            // 采样管理器
        tracer = 0x0A08,             // 链路追踪器
        profiler = 0x0A09,           // 性能分析器
        checker = 0x0A0A,            // 健康检查器
        reporter = 0x0A0B,           // 报告生成器
        anomaly_detector = 0x0A0C,   // 异常检测器
        threshold_manager = 0x0A0D,  // 阈值管理器
    };

}  // namespace error_system::module
