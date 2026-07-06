#pragma once
#include <memory>
#include <string>

#include "error_system/core/error_context.h"
#include "error_system/core/serializer/error_context_serializer.h"

/**
 * @file error_exception.h
 * @brief 错误异常类定义
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
     * @brief 错误异常类
     * @details 继承自 std::exception，适用于需要跨调用栈传播错误上下文的场景。
     *          内部用 shared_ptr 持有 error_context_t，满足 C++ 异常的可拷贝要求。
     */
    class error_exception_t : public std::exception {
    private:
        std::shared_ptr<const error_context_t> context_{};
        std::string cached_message_{};

    public:
        /**
         * @brief 从错误上下文构造异常（移动语义）
         * @details 参数按值传递以支持移动语义：调用方可通过 std::move(context) 避免拷贝。
         *          内部用 make_shared 转移所有权，零深拷贝。
         *          to_string 内部已捕获 bad_alloc，此处 try-catch 为额外防御，
         *          失败时 cached_message_ 保持空串，what() 返回空 C 字符串（非 nullptr）。
         * @param context 错误上下文（按值传递，支持移动）
         */
        explicit error_exception_t(error_context_t context) noexcept;

        error_exception_t(const error_exception_t&) = default;
        ~error_exception_t() noexcept override = default;
        error_exception_t& operator=(const error_exception_t&) = delete;
        error_exception_t(error_exception_t&&) noexcept = default;
        error_exception_t& operator=(error_exception_t&&) noexcept = delete;

        /**
         * @brief 实现 std::exception 接口
         * @return 完整的错误详情字符串
         */
        const char* what() const noexcept override { return cached_message_.c_str(); }

        /**
         * @brief 获取原始的错误上下文
         * @return 错误上下文引用，未持有时返回空哨兵
         */
        [[nodiscard]] const error_context_t& context() const noexcept {
            static const error_context_t empty{};
            return context_ ? *context_ : empty;
        }

        /**
         * @brief 获取原始错误码
         * @return 错误码，未持有时返回默认成功码
         */
        [[nodiscard]] error_code_t code() const noexcept {
            return context_ ? context_->get_code() : error_code_t{};
        }
    };

}  // namespace error_system::core
