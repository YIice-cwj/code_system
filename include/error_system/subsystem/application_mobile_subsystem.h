#pragma once
#include <cstdint>

/**
 * @file application_mobile_subsystem.h
 * @brief 应用移动段子系统分类定义
 * @details 定义应用移动段相关的子系统分类
 *          范围：0x0900 - 0x09FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 应用移动段子系统分类
     * @details 定义应用移动段相关的子系统，范围 0x0900 - 0x09FF
     */
    enum class application_mobile_subsystem_t : uint16_t {
        none = 0x0900,             // 无子系统
        ios_app = 0x0901,          // iOS应用
        android_app = 0x0902,      // Android应用
        flutter_app = 0x0903,      // Flutter应用
        react_native = 0x0904,     // ReactNative应用
        push_service = 0x0905,     // 推送服务
        offline_sync = 0x0906,     // 离线同步服务
        deep_link = 0x0907,        // 深度链接服务
        in_app_purchase = 0x0908,  // 应用内购买
    };

}  // namespace error_system::subsystem
