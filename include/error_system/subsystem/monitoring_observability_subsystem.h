#pragma once
#include <cstdint>

/**
 * @file monitoring_observability_subsystem.h
 * @brief 监控可观测性子系统分类定义
 * @details 定义监控可观测性相关的子系统分类
 *          范围：0x2000 - 0x20FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 监控可观测性子系统分类
     * @details 定义监控可观测性相关的子系统，范围 0x2000 - 0x20FF
     */
    enum class monitoring_observability_subsystem_t : uint16_t {
        none = 0x2000,           // 无子系统
        prometheus = 0x2001,     // Prometheus监控
        grafana = 0x2002,        // Grafana可视化
        jaeger = 0x2003,         // Jaeger链路追踪
        zipkin = 0x2004,         // Zipkin链路追踪
        skywalking = 0x2005,     // SkyWalking监控
        loki = 0x2006,           // Loki日志聚合
        elasticsearch = 0x2007,  // ES日志搜索
        fluentbit = 0x2008,      // Fluentbit采集
        opentelemetry = 0x2009,  // OpenTelemetry标准
    };

}  // namespace error_system::subsystem
