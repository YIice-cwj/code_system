#pragma once
#include <cstdint>

/**
 * @file middleware_module.h
 * @brief 中间件层功能模块定义
 * @details 定义中间件层相关的功能模块分类
 *          范围：0x0200 - 0x02FF（与中间件层系统域 0x02 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 中间件层功能模块分类
     * @details 定义中间件层相关的功能模块，范围 0x0200 - 0x02FF
     */
    enum class middleware_module_t : uint16_t {
        none = 0x0200,              // 无模块
        rpc_client = 0x0201,        // RPC客户端
        rpc_server = 0x0202,        // RPC服务端
        service_proxy = 0x0203,     // 服务代理
        service_stub = 0x0204,      // 服务存根
        api_gateway = 0x0205,       // API网关
        load_balancer = 0x0206,     // 负载均衡
        circuit_breaker = 0x0207,   // 熔断器
        rate_limiter = 0x0208,      // 限流器
        config_client = 0x0209,     // 配置客户端
        registry_client = 0x020A,   // 注册中心客户端
        tracing = 0x020B,           // 链路追踪
        metrics = 0x020C,           // 指标监控
        message_producer = 0x020D,  // 消息生产者
        message_consumer = 0x020E,  // 消息消费者
        message_broker = 0x020F,    // 消息代理
        cache_client = 0x0210,      // 缓存客户端
        cache_cluster = 0x0211,     // 缓存集群
        search_indexer = 0x0212,    // 搜索索引器
        search_query = 0x0213,      // 搜索查询器
        job_trigger = 0x0214,       // 任务触发器
        job_executor = 0x0215,      // 任务执行器
        id_generator = 0x0216,      // ID生成器
        lock_client = 0x0217,       // 锁客户端
        data_source = 0x0218,       // 数据源
        data_sink = 0x0219,         // 数据汇聚
    };

}  // namespace error_system::module
