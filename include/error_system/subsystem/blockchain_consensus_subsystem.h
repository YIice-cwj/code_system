#pragma once
#include <cstdint>

/**
 * @file blockchain_consensus_subsystem.h
 * @brief 区块链共识子系统分类定义
 * @details 定义区块链共识相关的子系统分类
 *          范围：0x1800 - 0x18FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 区块链共识子系统分类
     * @details 定义区块链共识相关的子系统，范围 0x1800 - 0x18FF
     */
    enum class blockchain_consensus_subsystem_t : uint16_t {
        none = 0x1800,        // 无子系统
        pow = 0x1801,         // 工作量证明
        pos = 0x1802,         // 权益证明
        pbft = 0x1803,        // 实用拜占庭容错
        raft = 0x1804,        // Raft共识
        hotstuff = 0x1805,    // HotStuff共识
        tendermint = 0x1806,  // Tendermint共识
        paxos = 0x1807,       // Paxos协议
    };

}  // namespace error_system::subsystem
