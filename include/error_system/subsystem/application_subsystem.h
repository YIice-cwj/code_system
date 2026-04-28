#pragma once
#include <cstdint>

/**
 * @file application_subsystem.h
 * @brief 应用层子系统分类定义
 * @details 定义应用层相关的子系统分类，用于标识错误码所属的应用层子系统
 *          范围：0x0300 - 0x03FF（与应用层系统域 0x03 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 应用层子系统分类
     * @details 定义应用层相关的子系统，范围 0x0300 - 0x03FF
     */
    enum class application_subsystem_t : uint16_t {
        none = 0x0300,         // 无子系统
        web = 0x0301,          // Web应用
        mobile = 0x0302,       // 移动应用
        desktop = 0x0303,      // 桌面应用
        cli = 0x0304,          // 命令行工具
        batch = 0x0305,        // 批处理任务
        workflow = 0x0306,     // 工作流引擎
        rule_engine = 0x0307,  // 规则引擎
    };

}  // namespace error_system::subsystem
