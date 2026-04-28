#pragma once
#include <cstdint>

/**
 * @file storage_subsystem.h
 * @brief 存储子系统分类定义
 * @details 定义存储相关的子系统分类，用于标识错误码所属的存储子系统
 *          范围：0x0600 - 0x06FF（与存储层系统域 0x06 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 存储子系统分类
     * @details 定义存储相关的子系统，范围 0x0600 - 0x06FF
     */
    enum class storage_subsystem_t : uint16_t {
        none = 0x0600,   // 无子系统
        local = 0x0601,  // 本地文件系统
        s3 = 0x0602,     // AWS S3对象存储
        oss = 0x0603,    // 阿里云OSS对象存储
        cos = 0x0604,    // 腾讯云COS对象存储
        hdfs = 0x0605,   // HDFS分布式文件系统
        nfs = 0x0606,    // NFS网络文件系统
        ceph = 0x0607,   // Ceph分布式存储
        minio = 0x0608,  // MinIO对象存储
    };

}  // namespace error_system::subsystem
