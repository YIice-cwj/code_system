#pragma once
#include <cstdint>

/**
 * @file cloud_module.h
 * @brief 云计算层功能模块定义
 * @details 定义云计算层相关的功能模块分类
 *          范围：0x0A00 - 0x0AFF（与云计算层系统域 0x0A 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 云计算层功能模块分类
     * @details 定义云计算层相关的功能模块，范围 0x0A00 - 0x0AFF
     */
    enum class cloud_module_t : uint16_t {
        none = 0x0A00,              // 无模块
        instance_manager = 0x0A01,  // 实例管理器
        image_manager = 0x0A02,     // 镜像管理器
        network_manager = 0x0A03,   // 网络管理器
        volume_manager = 0x0A04,    // 卷管理器
        snapshot_manager = 0x0A05,  // 快照管理器
        template_manager = 0x0A06,  // 模板管理器
        scheduler = 0x0A07,         // 调度器
        scaler = 0x0A08,            // 扩缩容器
        load_balancer = 0x0A09,     // 负载均衡器
        health_checker = 0x0A0A,    // 健康检查器
        metadata_service = 0x0A0B,  // 元数据服务
        tag_manager = 0x0A0C,       // 标签管理器
        quota_manager = 0x0A0D,     // 配额管理器
        billing_meter = 0x0A0E,     // 计费计量器
        event_notifier = 0x0A0F,    // 事件通知器
        api_controller = 0x0A10,    // API控制器
        policy_engine = 0x0A11,     // 策略引擎
    };

}  // namespace error_system::module
