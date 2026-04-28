#pragma once
#include <cstdint>

/**
 * @file middleware_subsystem.h
 * @brief 中间件层子系统分类定义
 * @details 定义中间件层相关的子系统分类，用于标识错误码所属的中间件层子系统
 *          范围：0x0200 - 0x02FF（与中间件层系统域 0x02 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 中间件层子系统分类
     * @details 定义中间件层相关的子系统，范围 0x0200 - 0x02FF
     */
    enum class middleware_subsystem_t : uint16_t {
        none = 0x0200,             // 无子系统
        rpc = 0x0201,              // RPC框架
        service_mesh = 0x0202,     // 服务网格
        api_gateway = 0x0203,      // API网关
        load_balancer = 0x0204,    // 负载均衡器
        config_center = 0x0205,    // 配置中心
        registry = 0x0206,         // 服务注册中心
        circuit_breaker = 0x0207,  // 熔断器
    };

}  // namespace error_system::subsystem
