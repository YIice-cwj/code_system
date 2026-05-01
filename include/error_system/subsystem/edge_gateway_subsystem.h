#pragma once
#include <cstdint>

/**
 * @file edge_gateway_subsystem.h
 * @brief 边缘网关子系统分类定义
 * @details 定义边缘网关相关的子系统分类
 *          范围：0x1400 - 0x14FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 边缘网关子系统分类
     * @details 定义边缘网关相关的子系统，范围 0x1400 - 0x14FF
     */
    enum class edge_gateway_subsystem_t : uint16_t {
        none = 0x1400,              // 无子系统
        protocol_adapter = 0x1401,  // 协议适配器
        data_aggregator = 0x1402,   // 数据聚合器
        local_cache = 0x1403,       // 本地缓存服务
        offline_buffer = 0x1404,    // 离线缓冲服务
        device_proxy = 0x1405,      // 设备代理服务
        rule_engine = 0x1406,       // 规则引擎
        stream_processor = 0x1407,  // 流处理器
    };

}  // namespace error_system::subsystem
