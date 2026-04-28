#pragma once
#include <cstdint>

/**
 * @file common_module.h
 * @brief 通用功能模块定义
 * @details 定义通用的功能模块分类，可被各子系统复用
 *          范围：0x0800 - 0x08FF（通用模块，独立于各系统域）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 通用功能模块分类
     * @details 定义通用的功能模块，范围 0x0800 - 0x08FF
     */
    enum class common_module_t : uint16_t {
        none = 0x0800,         // 无模块
        connector = 0x0801,    // 连接器
        parser = 0x0802,       // 解析器
        serializer = 0x0803,   // 序列化器
        validator = 0x0804,    // 验证器
        transformer = 0x0805,  // 转换器
        encoder = 0x0806,      // 编码器
        decoder = 0x0807,      // 解码器
        compressor = 0x0808,   // 压缩器
        encryptor = 0x0809,    // 加密器
        pool = 0x080A,         // 连接池
        cache = 0x080B,        // 缓存模块
        router = 0x080C,       // 路由器
        handler = 0x080D,      // 处理器
        interceptor = 0x080E,  // 拦截器
        filter = 0x080F,       // 过滤器
        adapter = 0x0810,      // 适配器
        proxy = 0x0811,        // 代理器
        balancer = 0x0812,     // 负载均衡器
        watcher = 0x0813,      // 监视器
        scheduler = 0x0814,    // 调度器
    };

}  // namespace error_system::module
