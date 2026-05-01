#pragma once
#include <cstdint>

/**
 * @file iot_device_subsystem.h
 * @brief 物联网设备子系统分类定义
 * @details 定义物联网设备相关的子系统分类
 *          范围：0x1600 - 0x16FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 物联网设备子系统分类
     * @details 定义物联网设备相关的子系统，范围 0x1600 - 0x16FF
     */
    enum class iot_device_subsystem_t : uint16_t {
        none = 0x1600,                 // 无子系统
        sensor_hub = 0x1601,           // 传感器中心
        actuator_controller = 0x1602,  // 执行器控制
        firmware_manager = 0x1603,     // 固件管理器
        device_registry = 0x1604,      // 设备注册中心
        lifecycle_manager = 0x1605,    // 生命周期管理
        shadow_manager = 0x1606,       // 设备影子管理
        command_dispatcher = 0x1607,   // 命令分发器
    };

}  // namespace error_system::subsystem
