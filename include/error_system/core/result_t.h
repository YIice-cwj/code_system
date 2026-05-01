#pragma once
#include <variant>
#include "error_system/core/error_context.h"

/**
 * @file result_t.h
 * @brief 结果数据类定义
 * @details 定义结果数据结构、字段解析和访问接口
 * @author yiice
 * @version 1.0.0
 * @date 2026-05-01
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 结果数据类
     * @details 封装结果信息，提供字段解析和访问功能
     * @tparam T 结果类型
     */
    template<typename T>
    class result_t {
        public:
        using value_type = T;

        private:
        std::variant<value_type, error_context_t> value_or_error_;

        public:
        /**
         * @brief 构造函数
         * @param value 成功值
         */
        result_t(const value_type& value) : value_or_error_(value) {}

        /**
         * @brief 移动构造函数
         * @param value 成功值
         */
        result_t(value_type&& value) : value_or_error_(std::move(value)) {}

        /**
         * @brief 构造函数
         * @param error_context 错误上下文
         */
        result_t(const error_context_t& error_context) : value_or_error_(error_context) {}
        
        /**
         * @brief 构造函数
         * @param code 错误码
         * @param message 错误信息
         */
        result_t(error_code_t code, const std::string& message = "") 
            : value_or_error_(error_context_t{code, message}) {}
        
        /**
         * @brief 检查结果是否为错误
         * @return bool 如果结果为错误则返回true
         */
        bool is_error() const noexcept { return std::holds_alternative<error_context_t>(value_or_error_); }
        
        /**
         * @brief 检查结果是否为成功
         * @return bool 如果结果为成功则返回true
         */
        bool is_success() const noexcept { return std::holds_alternative<value_type>(value_or_error_); }

        /**
         * @brief 获取错误上下文
         * @return const error_context_t& 错误上下文
         */
        const error_context_t& error() const noexcept { return std::get<error_context_t>(value_or_error_); }
        
        /**
         * @brief 获取成功值
         * @return const value_type& 成功值
         */
        const value_type& value() const noexcept { return std::get<value_type>(value_or_error_); }
        
        /**
         * @brief 获取成功值
         * @return value_type& 成功值
         */
        value_type& value() noexcept { return std::get<value_type>(value_or_error_); }
    };

    /** 
     * @brief 模板特化：当 T 为 void 时，特化为不包含值的 result_t
     * @details 特化 result_t 类模板，当 T 为 void 时，不存储值，仅存储错误上下文
     */
    template<>
    class result_t<void> {
        public:
        using value_type = void;

        private:
        error_context_t error_context_;

        public:
        /**
         * @brief 构造函数
         */
        result_t() : error_context_{} {}

        /**
         * @brief 构造函数
         * @param error_context 错误上下文
         */
        result_t(const error_context_t& error_context) : error_context_(error_context) {}
        
        /**
         * @brief 构造函数
         * @param code 错误码
         * @param message 错误信息
         */
        result_t(error_code_t code, const std::string& message = "") 
            : error_context_(error_context_t{code, message}) {}
        
        /**
         * @brief 检查结果是否为错误
         * @return bool 如果结果为错误则返回true
         */
        bool is_error() const noexcept { return static_cast<bool>(error_context_); }
        
        /**
         * @brief 检查结果是否为成功
         * @return bool 如果结果为成功则返回true
         */
        bool is_success() const noexcept { return !is_error(); }

        /**
         * @brief 获取错误上下文
         * @return const error_context_t& 错误上下文
         */
        const error_context_t& error() const noexcept { return error_context_; }
    };

}