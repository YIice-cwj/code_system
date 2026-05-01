#pragma once
#include <cstdint>

/**
 * @file security_module.h
 * @brief 安全防护功能模块定义
 * @details 定义安全防护相关的功能模块分类
 *          范围：0x0300 - 0x03FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 安全防护功能模块分类
     * @details 定义安全防护相关的功能模块，范围 0x0300 - 0x03FF
     */
    enum class security_module_t : uint16_t {
        none = 0x0300,                // 无模块
        authenticator = 0x0301,       // 身份认证器
        authorizer = 0x0302,          // 权限授权器
        encryptor = 0x0303,           // 数据加密器
        decryptor = 0x0304,           // 数据解密器
        hasher = 0x0305,              // 哈希计算器
        signer = 0x0306,              // 数字签名器
        verifier = 0x0307,            // 签名验证器
        certificate = 0x0308,         // 证书管理器
        firewall = 0x0309,            // 防火墙管理
        waf = 0x030A,                 // Web应用防护
        intrusion_detector = 0x030B,  // 入侵检测器
        auditor = 0x030C,             // 安全审计器
        token_manager = 0x030D,       // 令牌管理器
        session_manager = 0x030E,     // 会话管理器
        captcha = 0x030F,             // 验证码管理
        rate_limiter = 0x0310,        // 访问限流器
        access_control = 0x0311,      // 访问控制器
    };

}  // namespace error_system::module
