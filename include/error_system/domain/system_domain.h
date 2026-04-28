#pragma once
#include <cstdint>

/**
 * @file error_module.h
 * @brief 错误模块分类定义
 * @details 定义错误码系统中的模块层级结构，包括系统域、子系统和功能模块三级分类，
 *          用于64位错误码bit位分配中的模块标识，支持大规模分布式系统的错误码管理
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::domain {

    /**
     * @brief 系统域分类
     * @details 定义错误码系统中的系统域分类，用于标识错误码所属的系统域
     */
    enum class system_domain_t : uint8_t {
        system = 0,       // 系统层
        kernel = 1,       // 内核层
        middleware = 2,   // 中间件层
        application = 3,  // 应用层
        service = 4,      // 服务层
        network = 5,      // 网络层
        storage = 6,      // 存储层
        database = 7,     // 数据库层
    };

}  // namespace error_system::domain