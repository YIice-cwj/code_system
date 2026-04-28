#pragma once
#include <cstdint>

/**
 * @file service_module.h
 * @brief 服务层功能模块定义
 * @details 定义服务层相关的功能模块分类
 *          范围：0x0400 - 0x04FF（与服务层系统域 0x04 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 服务层功能模块分类
     * @details 定义服务层相关的功能模块，范围 0x0400 - 0x04FF
     */
    enum class service_module_t : uint16_t {
        none = 0x0400,                 // 无模块
        user_manager = 0x0401,         // 用户管理
        auth_manager = 0x0402,         // 认证管理
        session_manager = 0x0403,      // 会话管理
        order_manager = 0x0404,        // 订单管理
        payment_manager = 0x0405,      // 支付管理
        inventory_manager = 0x0406,    // 库存管理
        notification_sender = 0x0407,  // 通知发送
        search_engine = 0x0408,        // 搜索引擎
        recommender = 0x0409,          // 推荐引擎
        report_generator = 0x040A,     // 报表生成
        workflow_engine = 0x040B,      // 工作流引擎
        rule_engine = 0x040C,          // 规则引擎
        audit_logger = 0x040D,         // 审计日志
    };

}  // namespace error_system::module
