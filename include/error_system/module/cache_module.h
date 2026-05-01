#pragma once
#include <cstdint>

/**
 * @file cache_module.h
 * @brief 缓存管理功能模块定义
 * @details 定义缓存管理相关的功能模块分类
 *          范围：0x0500 - 0x05FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 缓存管理功能模块分类
     * @details 定义缓存管理相关的功能模块，范围 0x0500 - 0x05FF
     */
    enum class cache_module_t : uint16_t {
        none = 0x0500,               // 无模块
        local_cache = 0x0501,        // 本地缓存器
        distributed_cache = 0x0502,  // 分布式缓存
        memory_cache = 0x0503,       // 内存缓存器
        disk_cache = 0x0504,         // 磁盘缓存器
        lru_manager = 0x0505,        // LRU管理器
        lfu_manager = 0x0506,        // LFU管理器
        ttl_manager = 0x0507,        // TTL管理器
        eviction = 0x0508,           // 缓存淘汰器
        prefetcher = 0x0509,         // 缓存预取器
        warmer = 0x050A,             // 缓存预热器
        invalidator = 0x050B,        // 缓存失效器
        synchronizer = 0x050C,       // 缓存同步器
        serializer = 0x050D,         // 缓存序列化器
        compressor = 0x050E,         // 缓存压缩器
        partitioner = 0x050F,        // 缓存分区器
    };

}  // namespace error_system::module
