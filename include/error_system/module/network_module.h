#pragma once
#include <cstdint>

/**
 * @file network_module.h
 * @brief 网络功能模块定义
 * @details 定义网络相关的功能模块分类
 *          范围：0x0500 - 0x05FF（与网络层系统域 0x05 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 网络功能模块分类
     * @details 定义网络相关的功能模块，范围 0x0500 - 0x05FF
     */
    enum class network_module_t : uint16_t {
        none = 0x0500,            // 无模块
        socket = 0x0501,          // Socket管理
        listener = 0x0502,        // 监听器
        acceptor = 0x0503,        // 接收器
        connector = 0x0504,       // 连接器
        session = 0x0505,         // 会话管理
        channel = 0x0506,         // 通道管理
        pipeline = 0x0507,        // 协议管道处理
        handshake = 0x0508,       // 握手管理
        heartbeat = 0x0509,       // 心跳管理
        reconnection = 0x050A,    // 重连管理
        timeout = 0x050B,         // 超时处理
        dns_resolver = 0x050C,    // DNS解析器
        ssl_context = 0x050D,     // SSL上下文
        packet_encoder = 0x050E,  // 报文编码器
        packet_decoder = 0x050F,  // 报文解码器
        congestion = 0x0510,      // 拥塞控制
        flow_control = 0x0511,    // 流量控制
        nat = 0x0512,             // 网络地址转换
        acl = 0x0513,             // 访问控制列表
        qos = 0x0514,             // 服务质量
        multicast = 0x0515,       // 组播管理
        broadcast = 0x0516,       // 广播管理
        proxy_forward = 0x0517,   // 代理转发
        tunnel = 0x0518,          // 隧道管理
    };

}  // namespace error_system::module
