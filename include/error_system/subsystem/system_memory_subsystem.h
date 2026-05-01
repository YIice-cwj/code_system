#pragma once
#include <cstdint>

/**
 * @file system_memory_subsystem.h
 * @brief 系统内存子系统分类定义
 * @details 定义系统内存相关的子系统分类
 *          范围：0x0100 - 0x01FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 系统内存子系统分类
     * @details 定义系统内存相关的子系统，范围 0x0100 - 0x01FF
     */
    enum class system_memory_subsystem_t : uint16_t {
        none = 0x0100,            // 无子系统
        allocator = 0x0101,       // 内存分配器
        page_manager = 0x0102,    // 页面管理器
        swap_manager = 0x0103,    // 交换分区管理
        cache_manager = 0x0104,   // 缓存管理器
        buffer_manager = 0x0105,  // 缓冲区管理
        slab_allocator = 0x0106,  // Slab分配器
        vm_manager = 0x0107,      // 虚拟内存管理
        oom_handler = 0x0108,     // 内存溢出处理
        defragmenter = 0x0109,    // 内存整理器
    };

}  // namespace error_system::subsystem
