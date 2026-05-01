#pragma once
#include <cstdint>

/**
 * @file iot_network_subsystem.h
 * @brief 物联网网络子系统分类定义
 * @details 定义物联网网络相关的子系统分类
 *          范围：0x1700 - 0x17FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 物联网网络子系统分类
     * @details 定义物联网网络相关的子系统，范围 0x1700 - 0x17FF
     */
    enum class iot_network_subsystem_t : uint16_t {
        none = 0x1700,                // 无子系统
        mqtt_broker = 0x1701,         // MQTT代理服务
        coap_gateway = 0x1702,        // CoAP网关服务
        lorawan_server = 0x1703,      // LoRaWAN服务
        nb_iot_core = 0x1704,         // NB-IoT核心网
        zigbee_coordinator = 0x1705,  // ZigBee协调器
        ble_mesh = 0x1706,            // BLE网格网络
        thread_border = 0x1707,       // Thread边界路由
    };

}  // namespace error_system::subsystem
