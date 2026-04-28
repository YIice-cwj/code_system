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
    };

}  // namespace error_system::subsystem
