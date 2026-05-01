#pragma once
#include <cstdint>

/**
 * @file database_sql_subsystem.h
 * @brief 数据库SQL子系统分类定义
 * @details 定义数据库SQL相关的子系统分类
 *          范围：0x0C00 - 0x0CFF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 数据库SQL子系统分类
     * @details 定义数据库SQL相关的子系统，范围 0x0C00 - 0x0CFF
     */
    enum class database_sql_subsystem_t : uint16_t {
        none = 0x0C00,         // 无子系统
        mysql = 0x0C01,        // MySQL数据库
        postgresql = 0x0C02,   // PostgreSQL数据库
        sqlite = 0x0C03,       // SQLite数据库
        oracle = 0x0C04,       // Oracle数据库
        sqlserver = 0x0C05,    // SQLServer数据库
        mariadb = 0x0C06,      // MariaDB数据库
        cockroachdb = 0x0C07,  // CockroachDB数据库
    };

}  // namespace error_system::subsystem
