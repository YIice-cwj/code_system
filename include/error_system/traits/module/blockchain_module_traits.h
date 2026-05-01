#pragma once
#include "error_system/module/blockchain_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::blockchain_module_t, void> {

        using module_t = module::blockchain_module_t;
        using underlying_t = std::underlying_type_t<module_t>;

        /**
         * @brief 将枚举值转换为整数
         * @param module 枚举值
         * @return 对应的整数值
         */
        static constexpr underlying_t to_int(module_t module) noexcept {
            return static_cast<underlying_t>(module);
        }

        /**
         * @brief 校验整数是否为有效的枚举值
         * @param value 要校验的整数
         * @return 如果是有效的枚举值，返回 true；否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(module_t::none) && value <= to_int(module_t::rollup_operator);
        }

        /**
         * @brief 从整数转换为枚举值
         * @param value 要转换的整数
         * @return 对应的枚举值，如果无效则返回 module_t::none
         */
        static constexpr module_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return module_t::none;
            }
            return static_cast<module_t>(value);
        }

        /**
         * @brief 将枚举值转换为字符串
         * @param module 枚举值
         * @return 对应的字符串
         */
        static constexpr const char* to_string(module_t module) noexcept {
            switch (module) {
                case module_t::none:
                    return "none";
                case module_t::consensus_engine:
                    return "consensus_engine";
                case module_t::contract_executor:
                    return "contract_executor";
                case module_t::wallet_manager:
                    return "wallet_manager";
                case module_t::peer_manager:
                    return "peer_manager";
                case module_t::tx_pool:
                    return "tx_pool";
                case module_t::block_builder:
                    return "block_builder";
                case module_t::state_manager:
                    return "state_manager";
                case module_t::mempool_manager:
                    return "mempool_manager";
                case module_t::oracle_adapter:
                    return "oracle_adapter";
                case module_t::bridge_relayer:
                    return "bridge_relayer";
                case module_t::validator_set:
                    return "validator_set";
                case module_t::proposer:
                    return "proposer";
                case module_t::finality_gadget:
                    return "finality_gadget";
                case module_t::zk_prover:
                    return "zk_prover";
                case module_t::rollup_operator:
                    return "rollup_operator";
                default:
                    return "unknown";
            }
        }

        /**
         * @brief 从字符串转换为枚举值
         * @param string 要转换的字符串
         * @return 对应的枚举值，如果无效则返回 module_t::none
         */
        static constexpr module_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"):
                    return module_t::none;
                case utils::string_utils_t::hash("consensus_engine"):
                    return module_t::consensus_engine;
                case utils::string_utils_t::hash("contract_executor"):
                    return module_t::contract_executor;
                case utils::string_utils_t::hash("wallet_manager"):
                    return module_t::wallet_manager;
                case utils::string_utils_t::hash("peer_manager"):
                    return module_t::peer_manager;
                case utils::string_utils_t::hash("tx_pool"):
                    return module_t::tx_pool;
                case utils::string_utils_t::hash("block_builder"):
                    return module_t::block_builder;
                case utils::string_utils_t::hash("state_manager"):
                    return module_t::state_manager;
                case utils::string_utils_t::hash("mempool_manager"):
                    return module_t::mempool_manager;
                case utils::string_utils_t::hash("oracle_adapter"):
                    return module_t::oracle_adapter;
                case utils::string_utils_t::hash("bridge_relayer"):
                    return module_t::bridge_relayer;
                case utils::string_utils_t::hash("validator_set"):
                    return module_t::validator_set;
                case utils::string_utils_t::hash("proposer"):
                    return module_t::proposer;
                case utils::string_utils_t::hash("finality_gadget"):
                    return module_t::finality_gadget;
                case utils::string_utils_t::hash("zk_prover"):
                    return module_t::zk_prover;
                case utils::string_utils_t::hash("rollup_operator"):
                    return module_t::rollup_operator;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
