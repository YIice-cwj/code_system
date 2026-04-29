#pragma once
#include <cstdint>

/**
 * @file edge_module.h
 * @brief 边缘计算层功能模块定义
 * @details 定义边缘计算层相关的功能模块分类
 *          范围：0x0B00 - 0x0BFF（与边缘计算层系统域 0x0B 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 边缘计算层功能模块分类
     * @details 定义边缘计算层相关的功能模块，范围 0x0B00 - 0x0BFF
     */
    enum class edge_module_t : uint16_t {
        none = 0x0B00,                // 无模块
        gateway_controller = 0x0B01,  // 网关控制器
        node_agent = 0x0B02,          // 节点代理
        local_runtime = 0x0B03,       // 本地运行时
        cache_manager = 0x0B04,       // 缓存管理器
        sync_engine = 0x0B05,         // 同步引擎
        stream_processor = 0x0B06,    // 流处理器
        protocol_adapter = 0x0B07,    // 协议适配器
        device_connector = 0x0B08,    // 设备连接器
        rule_engine = 0x0B09,         // 规则引擎
        function_runner = 0x0B0A,     // 函数运行器
        message_router = 0x0B0B,      // 消息路由器
        data_filter = 0x0B0C,         // 数据过滤器
        offline_buffer = 0x0B0D,      // 离线缓冲区
        ota_manager = 0x0B0E,         // OTA管理器
    };

}  // namespace error_system::module
