#pragma once
#include <cstdint>

/**
 * @file common_module.h
 * @brief 通用功能模块定义
 * @details 定义通用的功能模块分类，可被各子系统复用
 *          范围：0x0B00 - 0x0BFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 通用功能模块分类
     * @details 定义通用的功能模块，范围 0x0B00 - 0x0BFF
     */
    enum class common_module_t : uint16_t {
        none = 0x0B00,          // 无模块
        serializer = 0x0B01,    // 数据序列化器
        deserializer = 0x0B02,  // 数据反序列化器
        validator = 0x0B03,     // 数据验证器
        transformer = 0x0B04,   // 数据转换器
        encoder = 0x0B05,       // 数据编码器
        decoder = 0x0B06,       // 数据解码器
        compressor = 0x0B07,    // 数据压缩器
        decompressor = 0x0B08,  // 数据解压缩器
        pool = 0x0B09,          // 通用连接池
        router = 0x0B0A,        // 通用路由器
        handler = 0x0B0B,       // 通用处理器
        interceptor = 0x0B0C,   // 通用拦截器
        adapter = 0x0B0D,       // 通用适配器
        balancer = 0x0B0E,      // 通用负载均衡
        throttler = 0x0B0F,     // 通用限流器
        retryer = 0x0B10,       // 通用重试器
        discovery = 0x0B11,     // 通用服务发现
        registry = 0x0B12,      // 通用服务注册
        event_bus = 0x0B13,     // 通用事件总线
        command_bus = 0x0B14,   // 通用命令总线
        query_bus = 0x0B15,     // 通用查询总线
        idempotency = 0x0B16,   // 通用幂等控制
        audit = 0x0B17,         // 通用审计记录
    };

}  // namespace error_system::module
