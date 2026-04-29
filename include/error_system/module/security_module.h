#pragma once
#include <cstdint>

/**
 * @file security_module.h
 * @brief 安全层功能模块定义
 * @details 定义安全层相关的功能模块分类
 *          范围：0x0800 - 0x08FF（与安全层系统域 0x08 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 安全层功能模块分类
     * @details 定义安全层相关的功能模块，范围 0x0800 - 0x08FF
     */
    enum class security_module_t : uint16_t {
        none = 0x0800,               // 无模块
        authenticator = 0x0801,      // 认证器
        authorizer = 0x0802,         // 授权器
        encryptor = 0x0803,          // 加密器
        decryptor = 0x0804,          // 解密器
        hash = 0x0805,               // 哈希计算
        signature = 0x0806,          // 数字签名
        certificate_manager = 0x0807,// 证书管理器
        key_store = 0x0808,          // 密钥存储
        token_generator = 0x0809,    // 令牌生成器
        token_validator = 0x080A,    // 令牌验证器
        session_manager = 0x080B,    // 会话管理器
        password_policy = 0x080C,    // 密码策略
        captcha = 0x080D,            // 验证码
        rate_limiter = 0x080E,       // 限流器
        ip_filter = 0x080F,          // IP过滤
        bot_detector = 0x0810,       // 机器人检测
        threat_intel = 0x0811,       // 威胁情报
        sandbox = 0x0812,            // 沙箱
    };

}  // namespace error_system::module
