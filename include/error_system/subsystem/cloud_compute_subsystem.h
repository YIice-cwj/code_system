#pragma once
#include <cstdint>

/**
 * @file cloud_compute_subsystem.h
 * @brief 云计算计算子系统分类定义
 * @details 定义云计算计算相关的子系统分类
 *          范围：0x1200 - 0x12FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 云计算计算子系统分类
     * @details 定义云计算计算相关的子系统，范围 0x1200 - 0x12FF
     */
    enum class cloud_compute_subsystem_t : uint16_t {
        none = 0x1200,               // 无子系统
        vm_manager = 0x1201,         // 虚拟机管理
        container_runtime = 0x1202,  // 容器运行时
        kubernetes = 0x1203,         // Kubernetes编排
        serverless = 0x1204,         // 无服务器计算
        auto_scaling = 0x1205,       // 自动扩缩容
        bare_metal = 0x1206,         // 裸金属管理
        gpu_cluster = 0x1207,        // GPU集群管理
        spot_instance = 0x1208,      // 竞价实例管理
    };

}  // namespace error_system::subsystem
