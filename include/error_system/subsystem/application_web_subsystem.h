#pragma once
#include <cstdint>

/**
 * @file application_web_subsystem.h
 * @brief 应用Web子系统分类定义
 * @details 定义应用Web相关的子系统分类
 *          范围：0x0800 - 0x08FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 应用Web子系统分类
     * @details 定义应用Web相关的子系统，范围 0x0800 - 0x08FF
     */
    enum class application_web_subsystem_t : uint16_t {
        none = 0x0800,             // 无子系统
        http_server = 0x0801,      // HTTP服务端
        http_client = 0x0802,      // HTTP客户端
        websocket = 0x0803,        // WebSocket服务
        rest_api = 0x0804,         // REST接口服务
        graphql = 0x0805,          // GraphQL服务
        template_engine = 0x0806,  // 模板引擎
        session_store = 0x0807,    // 会话存储
        cookie_manager = 0x0808,   // Cookie管理器
        cors_handler = 0x0809,     // 跨域处理器
        csrf_protector = 0x080A,   // CSRF防护器
        static_server = 0x080B,    // 静态资源服务
    };

}  // namespace error_system::subsystem
