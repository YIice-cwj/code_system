#pragma once
#include <cstdint>

/**
 * @file edge_subsystem.h
 * @brief 边缘计算层子系统分类定义
 * @details 定义边缘计算层相关的子系统分类，用于标识错误码所属的边缘计算层子系统
 *          范围：0x0B00 - 0x0BFF（与边缘计算层系统域 0x0B 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 边缘计算层子系统分类
     * @details 定义边缘计算层相关的子系统，范围 0x0B00 - 0x0BFF
     */
    enum class edge_subsystem_t : uint16_t {
        none = 0x0B00,              // 无子系统
        gateway = 0x0B01,           // 边缘网关
        node = 0x0B02,              // 边缘节点
        computing = 0x0B03,         // 边缘计算
        storage = 0x0B04,           // 边缘存储
        ai_inference = 0x0B05,      // 边缘AI推理
        video_analytics = 0x0B06,   // 边缘视频分析
        data_sync = 0x0B07,         // 边缘数据同步
        device_management = 0x0B08, // 设备管理
        fog = 0x0B09,               // 雾计算
        mec = 0x0B0A,               // 多接入边缘计算
        low_latency = 0x0B0B,       // 低延迟服务
        offline = 0x0B0C,           // 离线处理
        edge_ml = 0x0B0D,           // 边缘机器学习
    };

}  // namespace error_system::subsystem
