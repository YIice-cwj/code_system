#pragma once
#include <cstdint>

/**
 * @file cloud_subsystem.h
 * @brief 云计算层子系统分类定义
 * @details 定义云计算层相关的子系统分类，用于标识错误码所属的云计算层子系统
 *          范围：0x0A00 - 0x0AFF（与云计算层系统域 0x0A 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 云计算层子系统分类
     * @details 定义云计算层相关的子系统，范围 0x0A00 - 0x0AFF
     */
    enum class cloud_subsystem_t : uint16_t {
        none = 0x0A00,            // 无子系统
        compute = 0x0A01,         // 计算服务
        serverless = 0x0A02,      // 无服务器计算
        container = 0x0A03,       // 容器服务
        kubernetes = 0x0A04,      // Kubernetes编排
        vm = 0x0A05,              // 虚拟机
        bare_metal = 0x0A06,      // 裸金属服务器
        faas = 0x0A07,            // 函数即服务
        paas = 0x0A08,            // 平台即服务
        saas = 0x0A09,            // 软件即服务
        iaas = 0x0A0A,            // 基础设施即服务
        auto_scaling = 0x0A0B,    // 自动扩缩容
        load_balancing = 0x0A0C,  // 负载均衡
        cdn = 0x0A0D,             // 内容分发网络
        dns = 0x0A0E,             // 云DNS服务
        monitoring = 0x0A0F,      // 云监控
        logging = 0x0A10,         // 云日志服务
        iam = 0x0A11,             // 身份与访问管理
    };

}  // namespace error_system::subsystem
