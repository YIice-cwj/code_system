#pragma once
#include <cstdint>

/**
 * @file blockchain_contract_subsystem.h
 * @brief 区块链合约子系统分类定义
 * @details 定义区块链合约相关的子系统分类
 *          范围：0x1900 - 0x19FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 区块链合约子系统分类
     * @details 定义区块链合约相关的子系统，范围 0x1900 - 0x19FF
     */
    enum class blockchain_contract_subsystem_t : uint16_t {
        none = 0x1900,               // 无子系统
        evm = 0x1901,                // 以太坊虚拟机
        wasm_vm = 0x1902,            // WASM虚拟机
        solidity_compiler = 0x1903,  // Solidity编译器
        contract_deployer = 0x1904,  // 合约部署器
        oracle_network = 0x1905,     // 预言机网络
        bridge_relayer = 0x1906,     // 跨链中继器
    };

}  // namespace error_system::subsystem
