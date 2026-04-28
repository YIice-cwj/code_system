#pragma once
#include <cstdint>

/**
 * @file service_subsystem.h
 * @brief 服务层子系统分类定义
 * @details 定义服务层相关的子系统分类，用于标识错误码所属的服务层子系统
 *          范围：0x0400 - 0x04FF（与服务层系统域 0x04 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 服务层子系统分类
     * @details 定义服务层相关的子系统，范围 0x0400 - 0x04FF
     */
    enum class service_subsystem_t : uint16_t {
        none = 0x0400,            // 无子系统
        user = 0x0401,            // 用户服务
        auth = 0x0402,            // 认证服务
        order = 0x0403,           // 订单服务
        payment = 0x0404,         // 支付服务
        notification = 0x0405,    // 通知服务
        search = 0x0406,          // 搜索服务
        recommendation = 0x0407,  // 推荐服务
    };

}  // namespace error_system::subsystem
