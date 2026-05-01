#pragma once
#include <cstdint>

/**
 * @file service_microservice_subsystem.h
 * @brief 服务微服务子系统分类定义
 * @details 定义服务微服务相关的子系统分类
 *          范围：0x0A00 - 0x0AFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 服务微服务子系统分类
     * @details 定义服务微服务相关的子系统，范围 0x0A00 - 0x0AFF
     */
    enum class service_microservice_subsystem_t : uint16_t {
        none = 0x0A00,           // 无子系统
        gateway = 0x0A01,        // API网关服务
        registry = 0x0A02,       // 服务注册中心
        discovery = 0x0A03,      // 服务发现中心
        config_center = 0x0A04,  // 配置中心服务
        breaker = 0x0A05,        // 熔断器服务
        limiter = 0x0A06,        // 限流器服务
        balancer = 0x0A07,       // 负载均衡服务
        tracing = 0x0A08,        // 链路追踪服务
        metrics = 0x0A09,        // 指标采集服务
        health_check = 0x0A0A,   // 健康检查服务
    };

}  // namespace error_system::subsystem
