#pragma once
#include <cstdint>

/**
 * @file middleware_mq_subsystem.h
 * @brief 中间件消息队列子系统分类定义
 * @details 定义中间件消息队列相关的子系统分类
 *          范围：0x0700 - 0x07FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 中间件消息队列子系统分类
     * @details 定义中间件消息队列相关的子系统，范围 0x0700 - 0x07FF
     */
    enum class middleware_mq_subsystem_t : uint16_t {
        none = 0x0700,      // 无子系统
        kafka = 0x0701,     // Kafka消息队列
        rabbitmq = 0x0702,  // RabbitMQ队列
        rocketmq = 0x0703,  // RocketMQ队列
        pulsar = 0x0704,    // Pulsar消息队列
        nats = 0x0705,      // NATS消息系统
        zeromq = 0x0706,    // ZeroMQ队列
        activemq = 0x0707,  // ActiveMQ队列
        sns = 0x0708,       // 简单通知服务
        sqs = 0x0709,       // 简单队列服务
    };

}  // namespace error_system::subsystem
