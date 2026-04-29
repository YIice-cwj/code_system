#pragma once
#include <filesystem>

/**
 * @file file_util.h
 * @brief 文件工具
 * @details 定义文件相关的相关的函数，用于读取、写入、创建文件等
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief 读取文件内容
     * @details 从指定文件路径读取文件内容，返回文件内容的字符串表示
     * @param path 文件路径
     * @return std::string 文件内容的字符串表示
     */
    std::string read_file(const std::filesystem::path& path);

    /**
     * @brief 写入文件内容
     * @details 将指定字符串内容写入到指定文件路径
     * @param path 文件路径
     * @param content 要写入的字符串内容
     */
    void write_file(const std::filesystem::path& path, const std::string& content);

    /**
     * @brief 创建文件
     * @details 创建指定文件路径的文件，如果文件路径不存在则创建
     * @param path 文件路径
     */
    void create_file(const std::filesystem::path& path);

    /**
     * @brief 删除文件
     * @details 删除指定文件路径的文件
     * @param path 文件路径
     */
    void delete_file(const std::filesystem::path& path);

    /**
     * @brief 检查文件是否存在
     * @details 检查指定文件路径的文件是否存在
     * @param path 文件路径
     * @return bool 文件存在则返回 true，否则返回 false
     */
    bool file_exists(const std::filesystem::path& path);

    /**
     * @brief 检查文件路径是否存在
     * @details 检查指定文件路径是否存在
     * @param path 文件路径
     * @return bool 文件路径存在则返回 true，否则返回 false
     */
    bool file_path_exists(const std::filesystem::path& path);

}  // namespace error_system::utils
