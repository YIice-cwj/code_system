#pragma once
#include <cstdint>

/**
 * @file iot_module.h
 * @brief 物联网层功能模块定义
 * @details 定义物联网层相关的功能模块分类
 *          范围：0x0C00 - 0x0CFF（与物联网层系统域 0x0C 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 物联网层功能模块分类
     * @details 定义物联网层相关的功能模块，范围 0x0C00 - 0x0CFF
     */
    enum class iot_module_t : uint16_t {
        none = 0x0C00,               // 无模块
        device_registry = 0x0C01,    // 设备注册表
        sensor_reader = 0x0C02,      // 传感器读取器
        actuator_controller = 0x0C03,// 执行器控制器
        telemetry_collector = 0x0C04,// 遥测收集器
        command_dispatcher = 0x0C05, // 指令分发器
        firmware_updater = 0x0C06,   // 固件更新器
        twin_manager = 0x0C07,       // 孪生管理器
        broker_connector = 0x0C08,   // 代理连接器
        protocol_handler = 0x0C09,   // 协议处理器
        data_aggregator = 0x0C0A,    // 数据聚合器
        alarm_generator = 0x0C0B,    // 告警生成器
        geofence_engine = 0x0C0C,    // 地理围栏引擎
        energy_manager = 0x0C0D,     // 能耗管理器
        shadow_manager = 0x0C0E,     // 设备影子管理器
    };

}  // namespace error_system::module
