#pragma once
#include <cstdint>

/**
 * @file kernel_network_subsystem.h
 * @brief 内核网络子系统分类定义
 * @details 定义内核网络相关的子系统分类
 *          范围：0x0400 - 0x04FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 内核网络子系统分类
     * @details 定义内核网络相关的子系统，范围 0x0400 - 0x04FF
     */
    enum class kernel_network_subsystem_t : uint16_t {
        none = 0x0400,            // 无子系统
        netstack = 0x0401,        // 网络协议栈
        bridge = 0x0402,          // 网桥管理器
        vlan_manager = 0x0403,    // VLAN管理器
        netfilter = 0x0404,       // 包过滤框架
        tc_manager = 0x0405,      // 流量控制管理
        xdp_manager = 0x0406,     // XDP程序管理
        socket_manager = 0x0407,  // 套接字管理器
        routing_table = 0x0408,   // 路由表管理
        neighbor_table = 0x0409,  // 邻居表管理
    };

}  // namespace error_system::subsystem
