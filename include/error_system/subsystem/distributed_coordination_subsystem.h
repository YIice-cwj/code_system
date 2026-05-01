#pragma once
#include <cstdint>

/**
 * @file distributed_coordination_subsystem.h
 * @brief 分布式协调子系统分类定义
 * @details 定义分布式协调相关的子系统分类
 *          范围：0x1E00 - 0x1EFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 分布式协调子系统分类
     * @details 定义分布式协调相关的子系统，范围 0x1E00 - 0x1EFF
     */
    enum class distributed_coordination_subsystem_t : uint16_t {
        none = 0x1E00,              // 无子系统
        zookeeper = 0x1E01,         // ZooKeeper协调
        etcd_cluster = 0x1E02,      // Etcd集群协调
        consul = 0x1E03,            // Consul服务协调
        raft_cluster = 0x1E04,      // Raft集群协调
        gossip_protocol = 0x1E05,   // Gossip协议
        membership = 0x1E06,        // 成员管理
        leader_election = 0x1E07,   // 领导者选举
        distributed_lock = 0x1E08,  // 分布式锁
    };

}  // namespace error_system::subsystem
