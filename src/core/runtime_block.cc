#include "error_system/core/runtime_block.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "error_system/utils/object_pool.h"

/**
 * @file runtime_block.cc
 * @brief 动态运行时上下文堆块实现
 * @details 持有 error_context_t 的所有动态字段。按需分配：
 *          构造成功码或 make_minimal 时不分配（block_ 为 nullptr），
 *          仅在需要消息/payload/堆栈等动态数据时分配。
 *          字段为 public，仅供 error_context_t 及其友元
 *          （serializer/initializer）直接访问。
 *          内存由 utils::object_pool_t<runtime_block_t> 池化管理，
 *          deep_copy 从池中获取内存。
 * @author yiice
 * @version 4.3.0
 * @date 2026-07-07
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 深拷贝构造
     * @details 用于 error_context_t::clone() 和 wrap() 场景，完整复制所有字段
     *          （含 payload_overflow、raw_frames、resolved_frames 的独立副本）。
     *          内存从 utils::object_pool_t<runtime_block_t> 获取，
     *          通过 utils::pool_ptr_t<runtime_block_t> 持有所有权。
     *          字段拷贝可能抛出 std::bad_alloc，由调用方捕获；
     *          异常发生时 pool_ptr_t 析构将内存归还池，无泄漏。
     * @param other 源堆块
     * @return 深拷贝后的新堆块（pool_ptr_t 包装），池分配失败返回空 pool_ptr_t
     */
    utils::pool_ptr_t<runtime_block_t> runtime_block_t::deep_copy(const runtime_block_t& other) {
        utils::pool_ptr_t<runtime_block_t> block(utils::object_pool_t<runtime_block_t>::acquire());
        if (!block) {
            return block;
        }
        block->message = other.message;
        block->metadata = other.metadata;
        block->source_location = other.source_location;
        block->file_name = other.file_name;
        block->loc_file_storage = other.loc_file_storage;
        block->loc_func_storage = other.loc_func_storage;
        block->raw_frames = other.raw_frames;
        block->resolved_frames = other.resolved_frames;
        block->payload_count = other.payload_count;
        for (size_t i = 0; i < other.payload_count && i < other.payload_small.size(); ++i) {
            block->payload_small[i] = other.payload_small[i];
        }
        if (other.payload_overflow) {
            block->payload_overflow = std::make_unique<std::unordered_map<std::string, std::string>>(
                *other.payload_overflow);
        }
        return block;
    }

}  // namespace error_system::core
