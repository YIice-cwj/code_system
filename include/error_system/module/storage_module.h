#pragma once
#include <cstdint>

/**
 * @file storage_module.h
 * @brief 存储功能模块定义
 * @details 定义存储相关的功能模块分类
 *          范围：0x0600 - 0x06FF（与存储层系统域 0x06 对应）
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::module {

    /**
     * @brief 存储功能模块分类
     * @details 定义存储相关的功能模块，范围 0x0600 - 0x06FF
     */
    enum class storage_module_t : uint16_t {
        none = 0x0600,          // 无模块
        file_reader = 0x0601,   // 文件读取器
        file_writer = 0x0602,   // 文件写入器
        file_manager = 0x0603,  // 文件管理器
        directory = 0x0604,     // 目录管理
        watcher = 0x0605,       // 文件监视器
        uploader = 0x0606,      // 上传器
        downloader = 0x0607,    // 下载器
        sync = 0x0608,          // 同步模块
        backup = 0x0609,        // 备份模块
        compressor = 0x060A,    // 压缩模块
        encryptor = 0x060B,     // 加密模块
        bucket = 0x060C,        // 存储桶管理
        object = 0x060D,        // 对象管理
        multipart = 0x060E,     // 分片上传
    };

}  // namespace error_system::module
