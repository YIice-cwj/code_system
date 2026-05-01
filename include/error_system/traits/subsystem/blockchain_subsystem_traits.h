#pragma once
#include "error_system/subsystem/blockchain_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::blockchain_subsystem_t, void> {

        using subsystem_t = subsystem::blockchain_subsystem_t;
        using underlying_t = std::underlying_type_t<subsystem_t>;

        /**
         * @brief 将枚举值转换为整数
         * @param subsystem 枚举值
         * @return 对应的整数值
         */
        static constexpr underlying_t to_int(subsystem_t subsystem) noexcept {
            return static_cast<underlying_t>(subsystem);
        }

        /**
         * @brief 校验整数是否为有效的枚举值
         * @param value 要校验的整数
         * @return 如果是有效的枚举值，返回 true；否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::layer2);
        }

        /**
         * @brief 从整数转换为枚举值
         * @param value 要转换的整数
         * @return 对应的枚举值，如果无效则返回 subsystem_t::none
         */
        static constexpr subsystem_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return subsystem_t::none;
            }
            return static_cast<subsystem_t>(value);
        }

        /**
         * @brief 将枚举值转换为字符串
         * @param subsystem 枚举值
         * @return 对应的字符串
         */
        static constexpr const char* to_string(subsystem_t subsystem) noexcept {
            switch (subsystem) {
                case subsystem_t::none:
                    return "none";
                case subsystem_t::consensus:
                    return "consensus";
                case subsystem_t::smart_contract:
                    return "smart_contract";
                case subsystem_t::wallet:
                    return "wallet";
                case subsystem_t::node:
                    return "node";
                case subsystem_t::miner:
                    return "miner";
                case subsystem_t::validator:
                    return "validator";
                case subsystem_t::transaction:
                    return "transaction";
                case subsystem_t::block:
                    return "block";
                case subsystem_t::mempool:
                    return "mempool";
                case subsystem_t::oracle:
                    return "oracle";
                case subsystem_t::bridge:
                    return "bridge";
                case subsystem_t::defi:
                    return "defi";
                case subsystem_t::nft:
                    return "nft";
                case subsystem_t::dao:
                    return "dao";
                case subsystem_t::zk_proof:
                    return "zk_proof";
                case subsystem_t::layer2:
                    return "layer2";
                default:
                    return "unknown";
            }
        }

        /**
         * @brief 从字符串转换为枚举值
         * @param string 要转换的字符串
         * @return 对应的枚举值，如果无效则返回 subsystem_t::none
         */
        static constexpr subsystem_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"):
                    return subsystem_t::none;
                case utils::string_utils_t::hash("consensus"):
                    return subsystem_t::consensus;
                case utils::string_utils_t::hash("smart_contract"):
                    return subsystem_t::smart_contract;
                case utils::string_utils_t::hash("wallet"):
                    return subsystem_t::wallet;
                case utils::string_utils_t::hash("node"):
                    return subsystem_t::node;
                case utils::string_utils_t::hash("miner"):
                    return subsystem_t::miner;
                case utils::string_utils_t::hash("validator"):
                    return subsystem_t::validator;
                case utils::string_utils_t::hash("transaction"):
                    return subsystem_t::transaction;
                case utils::string_utils_t::hash("block"):
                    return subsystem_t::block;
                case utils::string_utils_t::hash("mempool"):
                    return subsystem_t::mempool;
                case utils::string_utils_t::hash("oracle"):
                    return subsystem_t::oracle;
                case utils::string_utils_t::hash("bridge"):
                    return subsystem_t::bridge;
                case utils::string_utils_t::hash("defi"):
                    return subsystem_t::defi;
                case utils::string_utils_t::hash("nft"):
                    return subsystem_t::nft;
                case utils::string_utils_t::hash("dao"):
                    return subsystem_t::dao;
                case utils::string_utils_t::hash("zk_proof"):
                    return subsystem_t::zk_proof;
                case utils::string_utils_t::hash("layer2"):
                    return subsystem_t::layer2;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
