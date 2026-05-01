#pragma once
#include <cstdint>

/**
 * @file network_module.h
 * @brief 网络通信功能模块定义
 * @details 定义网络通信相关的功能模块分类
 *          范围：0x0100 - 0x01FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 网络通信功能模块分类
     * @details 定义网络通信相关的功能模块，范围 0x0100 - 0x01FF
     */
    enum class network_module_t : uint16_t {
        none = 0x0100,               // 无模块
        socket = 0x0101,             // 套接字管理
        tcp_stack = 0x0102,          // TCP协议栈
        udp_stack = 0x0103,          // UDP协议栈
        http_client = 0x0104,        // HTTP客户端
        http_server = 0x0105,        // HTTP服务端
        websocket = 0x0106,          // WebSocket管理
        grpc_client = 0x0107,        // gRPC客户端
        grpc_server = 0x0108,        // gRPC服务端
        dns_resolver = 0x0109,       // DNS解析器
        load_balancer = 0x010A,      // 负载均衡器
        proxy = 0x010B,              // 网络代理器
        gateway = 0x010C,            // 网关管理器
        ssl_context = 0x010D,        // SSL上下文
        tls_handshake = 0x010E,      // TLS握手器
        connection_pool = 0x010F,    // 连接池管理
        packet_filter = 0x0110,      // 数据包过滤
        rate_limiter = 0x0111,       // 速率限制器
        circuit_breaker = 0x0112,    // 熔断器管理
        retryer = 0x0113,            // 重试机制器
        health_checker = 0x0114,     // 健康检查器
        service_discovery = 0x0115,  // 服务发现器
    };

}  // namespace error_system::module
