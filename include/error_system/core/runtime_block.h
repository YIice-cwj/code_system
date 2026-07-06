#pragma once
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "error_system/core/error_metadata.h"
#include "error_system/utils/source_location.h"

/**
 * @file runtime_block.h
 * @brief 动态运行时上下文堆块定义
 * @details 持有 error_context_t 的所有动态字段。按需分配：
 *          构造成功码或 make_minimal 时不分配（block_ 为 nullptr），
 *          仅在需要消息/payload/堆栈等动态数据时分配。
 *          字段为 public，仅供 error_context_t 及其友元
 *          （serializer/initializer）直接访问。
 * @author yiice
 * @version 4.0.0
 * @date 2026-07-06
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 动态运行时上下文堆块
     * @details 持有 error_context_t 的所有动态字段。
     *          payload 采用 SSO 优化：前 4 项内联存储，超出后溢出到 heap map。
     *          栈帧采用"延迟符号化"设计：
     *          - raw_frames 存储原始栈帧指针（capture 时存入，未符号化）
     *          - resolved_frames 存储已符号化的字符串（with_stack_frames 测试接口直接存入）
     *          二者互斥：生产路径用 raw_frames，输出时按需 resolve；测试路径用 resolved_frames。
     *          均使用 shared_ptr<const> 实现零拷贝共享。
     */
    struct runtime_block_t {
        std::string message{};
        std::optional<error_metadata_t> metadata{};
        utils::source_location_t source_location{};
        const char* file_name{nullptr};
        std::string loc_file_storage{};
        std::string loc_func_storage{};
        std::shared_ptr<const std::vector<void*>> raw_frames{};
        std::shared_ptr<const std::vector<std::string>> resolved_frames{};
        uint8_t payload_count{0};
        std::array<std::pair<std::string, std::string>, 4> payload_small{};
        std::unique_ptr<std::unordered_map<std::string, std::string>> payload_overflow{};

        runtime_block_t() = default;

        /**
         * @brief 深拷贝构造
         * @details 用于 error_context_t::clone() 和 wrap() 场景，完整复制所有字段
         *          （含 payload_overflow、raw_frames、resolved_frames 的独立副本）。
         * @param other 源堆块
         * @return 深拷贝后的新堆块
         */
        [[nodiscard]] static std::unique_ptr<runtime_block_t> deep_copy(const runtime_block_t& other);
    };

}  // namespace error_system::core
