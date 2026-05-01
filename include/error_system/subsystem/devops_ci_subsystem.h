#pragma once
#include <cstdint>

/**
 * @file devops_ci_subsystem.h
 * @brief 运维开发集成子系统分类定义
 * @details 定义运维开发集成相关的子系统分类
 *          范围：0x1C00 - 0x1CFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 运维开发集成子系统分类
     * @details 定义运维开发集成相关的子系统，范围 0x1C00 - 0x1CFF
     */
    enum class devops_ci_subsystem_t : uint16_t {
        none = 0x1C00,            // 无子系统
        jenkins = 0x1C01,         // Jenkins集成
        gitlab_ci = 0x1C02,       // GitLabCI集成
        github_actions = 0x1C03,  // GitHubActions
        tekton = 0x1C04,          // Tekton流水线
        argo_workflows = 0x1C05,  // Argo工作流
        drone = 0x1C06,           // Drone集成
        buildkite = 0x1C07,       // Buildkite集成
    };

}  // namespace error_system::subsystem
