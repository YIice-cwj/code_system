#pragma once
#include <cstdint>

/**
 * @file iot_subsystem.h
 * @brief 物联网层子系统分类定义
 * @details 定义物联网层相关的子系统分类，用于标识错误码所属的物联网层子系统
 *          范围：0x0C00 - 0x0CFF（与物联网层系统域 0x0C 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 物联网层子系统分类
     * @details 定义物联网层相关的子系统，范围 0x0C00 - 0x0CFF
     */
    enum class iot_subsystem_t : uint16_t {
        none = 0x0C00,              // 无子系统
        device = 0x0C01,            // 设备管理
        sensor = 0x0C02,            // 传感器
        actuator = 0x0C03,          // 执行器
        telemetry = 0x0C04,         // 遥测数据
        command = 0x0C05,           // 设备指令
        firmware = 0x0C06,          // 固件管理
        ota = 0x0C07,               // 空中升级
        digital_twin = 0x0C08,      // 数字孪生
        mqtt_broker = 0x0C09,       // MQTT代理
        lorawan = 0x0C0A,           // LoRaWAN网络
        nb_iot = 0x0C0B,            // NB-IoT网络
        zigbee_hub = 0x0C0C,        // ZigBee网关
        ble = 0x0C0D,               // 蓝牙低功耗
        rfid = 0x0C0E,              // 射频识别
        gps = 0x0C0F,               // 定位服务
        smart_home = 0x0C10,        // 智能家居
        industrial_iot = 0x0C11,    // 工业物联网
    };

}  // namespace error_system::subsystem
