#pragma once
#include <cstdint>

/**
 * @file devops_subsystem.h
 * @brief 运维开发层子系统分类定义
 * @details 定义运维开发层相关的子系统分类，用于标识错误码所属的运维开发层子系统
 *          范围：0x0F00 - 0x0FFF（与运维开发层系统域 0x0F 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 运维开发层子系统分类
     * @details 定义运维开发层相关的子系统，范围 0x0F00 - 0x0FFF
     */
    enum class devops_subsystem_t : uint16_t {
        none = 0x0F00,                // 无子系统
        ci = 0x0F01,                  // 持续集成
        cd = 0x0F02,                  // 持续部署
        pipeline = 0x0F03,            // 流水线
        build = 0x0F04,               // 构建系统
        test = 0x0F05,                // 自动化测试
        release = 0x0F06,             // 发布管理
        deploy = 0x0F07,              // 部署管理
        orchestration = 0x0F08,       // 编排管理
        monitoring = 0x0F09,          // 监控告警
        logging = 0x0F0A,             // 日志聚合
        tracing = 0x0F0B,             // 链路追踪
        apm = 0x0F0C,                 // 应用性能监控
        sre = 0x0F0D,                 // 站点可靠性工程
        chaos_engineering = 0x0F0E,   // 混沌工程
        gitops = 0x0F0F,              // GitOps
        iac = 0x0F10,                 // 基础设施即代码
        artifact = 0x0F11,            // 制品管理
    };

}  // namespace error_system::subsystem
