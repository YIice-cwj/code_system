#pragma once
#include <ostream>

#include "error_system/core/error_context.h"
#include "error_system/core/serializer/error_context_serializer.h"
#include "error_system/utils/log.h"

/**
 * @file error_formatter.h
 * @brief 错误格式化
 * @details 定义错误格式化相关的函数，包括 error_context_t 的输出流运算符重载
 * @author yiice
 * @version 3.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

/**
 * @brief 错误上下文输出流运算符重载
 * @param stream 输出流
 * @param context 错误上下文
 * @return std::ostream& 输出流
 */
[[nodiscard]] inline std::ostream& operator<<(std::ostream& stream, const error_context_t& context) noexcept {
    try {
        return stream << error_context_serializer_t::to_string(context);
    } catch (...) {
        LOG_ERROR("[error_formatter] operator<< threw exception");
        return stream;
    }
}

}  // namespace error_system::core
