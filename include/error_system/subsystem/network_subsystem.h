#pragma once
#include <cstdint>

/**
 * @file network_subsystem.h
 * @brief 网络子系统分类定义
 * @details 定义网络相关的子系统分类，用于标识错误码所属的网络子系统
 *          范围：0x0500 - 0x05FF（与网络层系统域 0x05 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 网络子系统分类
     * @details 定义网络相关的子系统，范围 0x0500 - 0x05FF
     */
    enum class network_subsystem_t : uint16_t {
        none = 0x0500,       // 无子系统
        tcp = 0x0501,        // TCP网络
        udp = 0x0502,        // UDP网络
        http = 0x0503,       // HTTP协议
        https = 0x0504,      // HTTPS协议
        websocket = 0x0505,  // WebSocket协议
        grpc = 0x0506,       // gRPC协议
        dns = 0x0507,        // DNS解析
        ssl_tls = 0x0508,    // SSL/TLS安全层
        ftp = 0x0509,        // FTP协议
        smtp = 0x050A,       // SMTP邮件协议
        icmp = 0x050B,       // ICMP协议
        quic = 0x050C,       // QUIC协议
        sctp = 0x050D,       // SCTP协议
        bluetooth = 0x050E,  // 蓝牙网络
        zigbee = 0x050F,     // ZigBee协议
        mqtt = 0x0510,       // MQTT协议
        coap = 0x0511,       // CoAP协议
        ntp = 0x0512,        // NTP时间同步
        radius = 0x0513,     // RADIUS认证
        vpn = 0x0514,        // VPN网络
        sdn = 0x0515,        // 软件定义网络
        cdn = 0x0516,        // 内容分发网络
        proxy = 0x0517,      // 代理服务
        firewall = 0x0518,   // 防火墙
    };

}  // namespace error_system::subsystem
