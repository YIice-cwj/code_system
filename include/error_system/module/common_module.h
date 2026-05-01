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
     * @details 定义通用的功能模块，范围 0xF000 - 0xFFFF
     */
    enum class common_module_t : uint16_t {
        none = 0xF000,             // 无模块
        connector = 0xF001,        // 连接器
        parser = 0xF002,           // 解析器
        serializer = 0xF003,       // 序列化器
        validator = 0xF004,        // 验证器
        transformer = 0xF005,      // 转换器
        encoder = 0xF006,          // 编码器
        decoder = 0xF007,          // 解码器
        compressor = 0xF008,       // 压缩器
        encryptor = 0xF009,        // 加密器
        pool = 0xF00A,             // 连接池
        cache = 0xF00B,            // 缓存模块
        router = 0xF00C,           // 路由器
        handler = 0xF00D,          // 处理器
        interceptor = 0xF00E,      // 拦截器
        filter = 0xF00F,           // 过滤器
        adapter = 0xF010,          // 适配器
        proxy = 0xF011,            // 代理器
        balancer = 0xF012,         // 负载均衡器
        watcher = 0xF013,          // 监视器
        scheduler = 0xF014,        // 调度器
        throttler = 0xF015,        // 限流器
        retryer = 0xF016,          // 重试器
        circuit_breaker = 0xF017,  // 熔断器
        health_checker = 0xF018,   // 健康检查器
        discovery = 0xF019,        // 服务发现
        registry = 0xF01A,         // 服务注册
        config_loader = 0xF01B,    // 配置加载器
        event_bus = 0xF01C,        // 事件总线
        command_bus = 0xF01D,      // 命令总线
        query_bus = 0xF01E,        // 查询总线
        saga = 0xF01F,             // Saga协调器
        outbox = 0xF020,           // 发件箱模式
        inbox = 0xF021,            // 收件箱模式
        idempotency = 0xF022,      // 幂等性控制
        audit = 0xF023,            // 审计记录器
        meter = 0xF024,            // 度量收集器
        tracer = 0xF025,           // 链路追踪器
        logger = 0xF026,           // 日志记录器
    };

}  // namespace error_system::module
