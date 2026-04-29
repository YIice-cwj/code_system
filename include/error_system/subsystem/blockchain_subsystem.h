#pragma once
#include <cstdint>

/**
 * @file blockchain_subsystem.h
 * @brief 区块链层子系统分类定义
 * @details 定义区块链层相关的子系统分类，用于标识错误码所属的区块链层子系统
 *          范围：0x0D00 - 0x0DFF（与区块链层系统域 0x0D 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 区块链层子系统分类
     * @details 定义区块链层相关的子系统，范围 0x0D00 - 0x0DFF
     */
    enum class blockchain_subsystem_t : uint16_t {
        none = 0x0D00,            // 无子系统
        consensus = 0x0D01,       // 共识机制
        smart_contract = 0x0D02,  // 智能合约
        wallet = 0x0D03,          // 钱包管理
        node = 0x0D04,            // 区块链节点
        miner = 0x0D05,           // 矿工
        validator = 0x0D06,       // 验证者
        transaction = 0x0D07,     // 交易处理
        block = 0x0D08,           // 区块管理
        mempool = 0x0D09,         // 内存池
        oracle = 0x0D0A,          // 预言机
        bridge = 0x0D0B,          // 跨链桥
        defi = 0x0D0C,            // 去中心化金融
        nft = 0x0D0D,             // 非同质化代币
        dao = 0x0D0E,             // 去中心化自治组织
        zk_proof = 0x0D0F,        // 零知识证明
        layer2 = 0x0D10,          // 二层网络
    };

}  // namespace error_system::subsystem
