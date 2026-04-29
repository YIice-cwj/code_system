#pragma once
#include <cstdint>

/**
 * @file devops_module.h
 * @brief 运维开发层功能模块定义
 * @details 定义运维开发层相关的功能模块分类
 *          范围：0x0F00 - 0x0FFF（与运维开发层系统域 0x0F 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 运维开发层功能模块分类
     * @details 定义运维开发层相关的功能模块，范围 0x0F00 - 0x0FFF
     */
    enum class devops_module_t : uint16_t {
        none = 0x0F00,               // 无模块
        scm_connector = 0x0F01,      // 源码管理连接器
        build_executor = 0x0F02,     // 构建执行器
        test_runner = 0x0F03,        // 测试运行器
        package_builder = 0x0F04,    // 包构建器
        deploy_engine = 0x0F05,      // 部署引擎
        rollback_manager = 0x0F06,   // 回滚管理器
        environment_manager = 0x0F07,// 环境管理器
        secret_manager = 0x0F08,     // 密钥管理器
        alert_manager = 0x0F09,      // 告警管理器
        log_aggregator = 0x0F0A,     // 日志聚合器
        trace_collector = 0x0F0B,    // 追踪收集器
        metric_scraper = 0x0F0C,     // 指标抓取器
        slo_calculator = 0x0F0D,     // SLO计算器
        chaos_injector = 0x0F0E,     // 混沌注入器
        git_syncer = 0x0F0F,         // Git同步器
        provisioner = 0x0F10,        // 配置器
        artifact_registry = 0x0F11,  // 制品仓库
    };

}  // namespace error_system::module
