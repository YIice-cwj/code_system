#pragma once
#include <cstdint>

/**
 * @file message_module.h
 * @brief 消息队列功能模块定义
 * @details 定义消息队列相关的功能模块分类
 *          范围：0x0600 - 0x06FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 消息队列功能模块分类
     * @details 定义消息队列相关的功能模块，范围 0x0600 - 0x06FF
     */
    enum class message_module_t : uint16_t {
        none = 0x0600,             // 无模块
        producer = 0x0601,         // 消息生产者
        consumer = 0x0602,         // 消息消费者
        broker = 0x0603,           // 消息代理器
        exchange = 0x0604,         // 消息交换器
        queue_manager = 0x0605,    // 队列管理器
        topic_manager = 0x0606,    // 主题管理器
        partitioner = 0x0607,      // 消息分区器
        router = 0x0608,           // 消息路由器
        serializer = 0x0609,       // 消息序列化器
        deserializer = 0x060A,     // 消息反序列化器
        compressor = 0x060B,       // 消息压缩器
        idempotency = 0x060C,      // 幂等性控制
        deduplicator = 0x060D,     // 消息去重器
        delay_scheduler = 0x060E,  // 延迟调度器
        dead_letter = 0x060F,      // 死信管理器
        transaction = 0x0610,      // 消息事务器
        ack_manager = 0x0611,      // 确认管理器
    };

}  // namespace error_system::module
