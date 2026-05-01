#pragma once
#include <cstdint>

/**
 * @file devops_cd_subsystem.h
 * @brief 运维开发部署子系统分类定义
 * @details 定义运维开发部署相关的子系统分类
 *          范围：0x1D00 - 0x1DFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 运维开发部署子系统分类
     * @details 定义运维开发部署相关的子系统，范围 0x1D00 - 0x1DFF
     */
    enum class devops_cd_subsystem_t : uint16_t {
        none = 0x1D00,       // 无子系统
        spinnaker = 0x1D01,  // Spinnaker部署
        argo_cd = 0x1D02,    // ArgoCD部署
        flux_cd = 0x1D03,    // FluxCD部署
        helm = 0x1D04,       // Helm包管理
        kustomize = 0x1D05,  // Kustomize配置
        terraform = 0x1D06,  // Terraform编排
        ansible = 0x1D07,    // Ansible配置
        puppet = 0x1D08,     // Puppet配置
        chef = 0x1D09,       // Chef配置
    };

}  // namespace error_system::subsystem
