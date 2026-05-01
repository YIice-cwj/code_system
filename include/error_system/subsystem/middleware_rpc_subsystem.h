#pragma once
#include <cstdint>

/**
 * @file middleware_rpc_subsystem.h
 * @brief 中间件RPC子系统分类定义
 * @details 定义中间件RPC相关的子系统分类
 *          范围：0x0600 - 0x06FF
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::subsystem {

    /**
     * @brief 中间件RPC子系统分类
     * @details 定义中间件RPC相关的子系统，范围 0x0600 - 0x06FF
     */
    enum class middleware_rpc_subsystem_t : uint16_t {
        none = 0x0600,          // 无子系统
        grpc = 0x0601,          // gRPC框架
        thrift = 0x0602,        // Thrift框架
        dubbo = 0x0603,         // Dubbo框架
        brpc = 0x0604,          // bRPC框架
        tars = 0x0605,          // Tars框架
        sofa_rpc = 0x0606,      // SOFARPC框架
        service_mesh = 0x0607,  // 服务网格
        sidecar = 0x0608,       // Sidecar代理
    };

}  // namespace error_system::subsystem
