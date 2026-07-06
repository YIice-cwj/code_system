#include "error_system/core/error_exception.h"

#include <memory>
#include <string>
#include <utility>

#include "error_system/core/serializer/error_context_serializer.h"

/**
 * @file error_exception.cc
 * @brief 错误异常类实现
 * @details 将 error_context_t 封装为可抛出的 std::exception 派生异常。
 *          error_context_t 为 Move-Only 后，error_exception_t 内部用
 *          shared_ptr<error_context_t> 持有，保持 std::exception 派生类的可拷贝要求
 *          （C++ 异常机制约束）。构造时缓存错误详情字符串，what() 返回该缓存。
 * @author yiice
 * @version 4.0.0
 * @date 2026-07-06
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 从错误上下文构造异常（移动语义）
     * @details 参数按值传递以支持移动语义：调用方可通过 std::move(context) 避免拷贝。
     *          内部用 make_shared 转移所有权，零深拷贝。
     *          to_string 内部已捕获 bad_alloc，此处 try-catch 为额外防御，
     *          失败时 cached_message_ 保持空串，what() 返回空 C 字符串（非 nullptr）。
     * @param context 错误上下文（按值传递，支持移动）
     */
    error_exception_t::error_exception_t(error_context_t context) noexcept {
        try {
            context_ = std::make_shared<const error_context_t>(std::move(context));
            if (context_) {
                cached_message_ = error_context_serializer_t::to_string(*context_);
            }
        } catch (const std::bad_alloc&) {
            cached_message_.clear();
        }
    }

}  // namespace error_system::core
