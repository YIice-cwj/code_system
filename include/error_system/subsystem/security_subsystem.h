#pragma once
#include <cstdint>

/**
 * @file security_subsystem.h
 * @brief 安全层子系统分类定义
 * @details 定义安全层相关的子系统分类，用于标识错误码所属的安全层子系统
 *          范围：0x0800 - 0x08FF（与安全层系统域 0x08 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 安全层子系统分类
     * @details 定义安全层相关的子系统，范围 0x0800 - 0x08FF
     */
    enum class security_subsystem_t : uint16_t {
        none = 0x0800,                 // 无子系统
        auth = 0x0801,                 // 身份认证
        authorization = 0x0802,        // 访问授权
        encryption = 0x0803,           // 数据加密
        firewall = 0x0804,             // 网络防火墙
        intrusion_detection = 0x0805,  // 入侵检测
        vulnerability_scan = 0x0806,   // 漏洞扫描
        certificate = 0x0807,          // 证书管理
        key_management = 0x0808,       // 密钥管理
        sso = 0x0809,                  // 单点登录
        mfa = 0x080A,                  // 多因素认证
        oauth = 0x080B,                // OAuth认证
        ldap = 0x080C,                 // LDAP目录服务
        hsm = 0x080D,                  // 硬件安全模块
        siem = 0x080E,                 // 安全信息与事件管理
        zero_trust = 0x080F,           // 零信任架构
        waf = 0x0810,                  // Web应用防火墙
        ddos_protection = 0x0811,      // DDoS防护
    };

}  // namespace error_system::subsystem
