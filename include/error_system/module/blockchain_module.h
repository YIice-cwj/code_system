#pragma once
#include <cstdint>

/**
 * @file blockchain_module.h
 * @brief 区块链层功能模块定义
 * @details 定义区块链层相关的功能模块分类
 *          范围：0x0D00 - 0x0DFF（与区块链层系统域 0x0D 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 区块链层功能模块分类
     * @details 定义区块链层相关的功能模块，范围 0x0D00 - 0x0DFF
     */
    enum class blockchain_module_t : uint16_t {
        none = 0x0D00,               // 无模块
        consensus_engine = 0x0D01,   // 共识引擎
        contract_executor = 0x0D02,  // 合约执行器
        wallet_manager = 0x0D03,     // 钱包管理器
        peer_manager = 0x0D04,       // 对等节点管理器
        tx_pool = 0x0D05,            // 交易池
        block_builder = 0x0D06,      // 区块构建器
        state_manager = 0x0D07,      // 状态管理器
        mempool_manager = 0x0D08,    // 内存池管理器
        oracle_adapter = 0x0D09,     // 预言机适配器
        bridge_relayer = 0x0D0A,     // 跨链中继器
        validator_set = 0x0D0B,      // 验证者集合
        proposer = 0x0D0C,           // 提案者
        finality_gadget = 0x0D0D,    // 最终性工具
        zk_prover = 0x0D0E,          // 零知识证明器
        rollup_operator = 0x0D0F,    // Rollup操作器
    };

}  // namespace error_system::module
