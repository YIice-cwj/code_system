#pragma once
#include "error_system/core/error_code.h"

/**
 * @file application_error.h
 * @brief 应用层错误码定义
 * @details 定义应用层相关的错误码常量，使用 core::code_t 类型
 * @author yiice
 * @version 1.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::error_codes {
    /**
     * @brief 未知错误
     */
    constexpr core::code_t APPLICATION_UNKNOWN_ERROR = 0x8000000003000001ULL;

    /**
     * @brief 无效参数
     */
    constexpr core::code_t APPLICATION_INVALID_PARAMETER = 0x8000000003000002ULL;

    /**
     * @brief 资源未找到
     */
    constexpr core::code_t APPLICATION_RESOURCE_NOT_FOUND = 0x8000000003000003ULL;

    /**
     * @brief 操作不允许
     */
    constexpr core::code_t APPLICATION_OPERATION_NOT_ALLOWED = 0x8000000003000004ULL;

    /**
     * @brief 路由未找到
     */
    constexpr core::code_t APPLICATION_CONTROLLER_ROUTE_NOT_FOUND = 0x8000030003000100ULL;

    /**
     * @brief 请求方法不允许
     */
    constexpr core::code_t APPLICATION_CONTROLLER_METHOD_NOT_ALLOWED = 0x8000030003000101ULL;

    /**
     * @brief 业务逻辑错误
     */
    constexpr core::code_t APPLICATION_SERVICE_BUSINESS_ERROR = 0x8000030003000200ULL;

    /**
     * @brief 服务不可用
     */
    constexpr core::code_t APPLICATION_SERVICE_UNAVAILABLE = 0x8000030003000201ULL;

    /**
     * @brief 数据格式错误
     */
    constexpr core::code_t APPLICATION_VALIDATOR_FORMAT_ERROR = 0x8000030003000300ULL;

    /**
     * @brief 数据为空
     */
    constexpr core::code_t APPLICATION_VALIDATOR_EMPTY_DATA = 0x8000030003000301ULL;

}  // namespace error_system::error_codes
