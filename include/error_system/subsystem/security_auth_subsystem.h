#pragma once
#include <cstdint>

/**
 * @file security_auth_subsystem.h
 * @brief 安全认证子系统分类定义
 * @details 定义安全认证相关的子系统分类
 *          范围：0x0E00 - 0x0EFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 安全认证子系统分类
     * @details 定义安全认证相关的子系统，范围 0x0E00 - 0x0EFF
     */
    enum class security_auth_subsystem_t : uint16_t {
        none = 0x0E00,      // 无子系统
        oauth2 = 0x0E01,    // OAuth2认证
        sso = 0x0E02,       // 单点登录
        ldap = 0x0E03,      // LDAP目录服务
        saml = 0x0E04,      // SAML断言
        jwt = 0x0E05,       // JWT令牌
        mfa = 0x0E06,       // 多因素认证
        kerberos = 0x0E07,  // Kerberos认证
        cas = 0x0E08,       // CAS认证中心
    };

}  // namespace error_system::subsystem
