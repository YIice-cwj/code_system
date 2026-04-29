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
        message_queue = 0x0208,    // 消息队列
        cache = 0x0209,            // 分布式缓存
        search_engine = 0x020A,    // 搜索引擎中间件
        job_scheduler = 0x020B,    // 任务调度中间件
        data_pipeline = 0x020C,    // 数据管道
        id_generator = 0x020D,     // 分布式ID生成器
        lock_service = 0x020E,     // 分布式锁服务
        kafka = 0x020F,            // Kafka消息队列
        rabbitmq = 0x0210,         // RabbitMQ消息队列
        rocketmq = 0x0211,         // RocketMQ消息队列
        pulsar = 0x0212,           // Pulsar消息队列
        nats = 0x0213,             // NATS消息队列
        memcached = 0x0214,        // Memcached缓存
        zookeeper = 0x0215,        // Zookeeper协调服务
        consul = 0x0216,           // Consul服务发现
        nginx = 0x0217,            // Nginx网关
        haproxy = 0x0218,          // HAProxy负载均衡
        envoy = 0x0219,            // Envoy代理
        istio = 0x021A,            // Istio服务网格
        dapr = 0x021B,             // Dapr运行时
        seata = 0x021C,            // Seata分布式事务
        shardingsphere = 0x021D,   // ShardingSphere分库分表
    };

}  // namespace error_system::subsystem
