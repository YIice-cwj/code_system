#pragma once
#include <cstdint>

/**
 * @file security_crypto_subsystem.h
 * @brief 安全加密子系统分类定义
 * @details 定义安全加密相关的子系统分类
 *          范围：0x0F00 - 0x0FFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 安全加密子系统分类
     * @details 定义安全加密相关的子系统，范围 0x0F00 - 0x0FFF
     */
    enum class security_crypto_subsystem_t : uint16_t {
        none = 0x0F00,          // 无子系统
        tls_manager = 0x0F01,   // TLS管理器
        cert_manager = 0x0F02,  // 证书管理器
        hsm = 0x0F03,           // 硬件安全模块
        key_vault = 0x0F04,     // 密钥保险箱
        secret_manager = 0x0F05, // 密钥管理器
        hash_engine = 0x0F06,   // 哈希引擎
        cipher_engine = 0x0F07, // 加密引擎
        pkix = 0x0F08,          // PKIX标准
    };

}  // namespace error_system::subsystem
