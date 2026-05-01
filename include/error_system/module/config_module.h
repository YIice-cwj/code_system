#pragma once
#include <cstdint>

/**
 * @file config_module.h
 * @brief 配置管理功能模块定义
 * @details 定义配置管理相关的功能模块分类
 *          范围：0x0000 - 0x00FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 配置管理功能模块分类
     * @details 定义配置管理相关的功能模块，范围 0x0000 - 0x00FF
     */
    enum class config_module_t : uint16_t {
        none = 0x0000,          // 无模块
        loader = 0x0001,        // 配置加载器
        parser = 0x0002,        // 配置解析器
        validator = 0x0003,     // 配置验证器
        merger = 0x0004,        // 配置合并器
        watcher = 0x0005,       // 配置监视器
        reloader = 0x0006,      // 配置重载器
        encryptor = 0x0007,     // 配置加密器
        decryptor = 0x0008,     // 配置解密器
        templater = 0x0009,     // 配置模板器
        interpolator = 0x000A,  // 配置插值器
        schema = 0x000B,        // 配置模式器
        migrator = 0x000C,      // 配置迁移器
        differ = 0x000D,        // 配置对比器
        snapshot = 0x000E,      // 配置快照器
        rollbacker = 0x000F,    // 配置回滚器
    };

}  // namespace error_system::module
