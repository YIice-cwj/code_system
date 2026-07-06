#pragma once

#include "error_system/async/async_result.h"

#include <chrono>
#include <cstdio>
#include <future>
#include <new>
#include <type_traits>
#include <utility>

/**
 * @file async_result.inl
 * @brief async_result_t 模板实现
 * @author yiice
 * @version 3.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */
namespace error_system::async {

    namespace detail {
        /**
         * @brief 构造异步链中回调抛出异常时的 fatal 错误上下文
         * @param message 错误消息
         * @return core::error_context_t fatal 错误上下文
         */
        inline core::error_context_t make_async_exception_context(const char* message) noexcept {
            try {
                return core::error_context_t{
                    core::located_code_t{core::error_code_t(core::error_level_t::fatal,
                                                             domain::system_domain_t::none,
                                                             core::subsystem_id_t{0},
                                                             core::module_id_t{0},
                                                             core::error_number_t{0xFFFE})},
                    message};
            } catch (const std::bad_alloc&) {
                std::fprintf(stderr, "[async_result] make_async_exception_context: std::bad_alloc\n");
                static thread_local core::error_context_t fallback{};
                return fallback.clone();
            }
        }
    }  // namespace detail

    template <typename T, bool Lean>
    async_result_t<T, Lean>::async_result_t(std::future<result_type_t> future) noexcept
        : future_(std::move(future)) {}

    template <typename T, bool Lean>
    async_result_t<T, Lean>::async_result_t(result_type_t result) noexcept
        : future_(std::async(std::launch::async,
                             [captured_result = std::move(result)]() mutable { return std::move(captured_result); })) {}

    template <typename T, bool Lean>
    auto async_result_t<T, Lean>::get() -> result_type_t {
        try {
            return future_.get();
        } catch (...) {
            std::fprintf(stderr, "[async_result] get: inner future threw exception\n");
            return result_type_t(detail::make_async_exception_context("async get: inner future threw"));
        }
    }

    template <typename T, bool Lean>
    void async_result_t<T, Lean>::wait() const noexcept {
        if (future_.valid()) {
            future_.wait();
        }
    }

    template <typename T, bool Lean>
    bool async_result_t<T, Lean>::is_ready() const noexcept {
        if (!future_.valid()) {
            return false;
        }
        return future_.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

    template <typename T, bool Lean>
    bool async_result_t<T, Lean>::valid() const noexcept {
        return future_.valid();
    }

    template <typename T, bool Lean>
    template <typename Function>
    auto async_result_t<T, Lean>::then(Function&& function) && -> async_result_t<result_value_t<std::invoke_result_t<Function, result_type_t>>, Lean> {
        using next_result_t = std::invoke_result_t<Function, result_type_t>;
        using next_value_t = result_value_t<next_result_t>;

        return async_result_t<next_value_t, Lean>(
            std::async(std::launch::async,
                       [captured_future = std::move(future_), captured_function = std::forward<Function>(function)]() mutable -> next_result_t {
                           try {
                               result_type_t input = captured_future.get();
                               return captured_function(std::move(input));
                           } catch (...) {
                               std::fprintf(stderr, "[async_result] then: threw exception\n");
                               return next_result_t(detail::make_async_exception_context("async then: function threw"));
                           }
                       }));
    }

    template <typename T, bool Lean>
    template <typename Function>
    async_result_t<T, Lean> async_result_t<T, Lean>::recover(Function&& function) && {
        return async_result_t<T, Lean>(
            std::async(std::launch::async,
                       [captured_future = std::move(future_), captured_function = std::forward<Function>(function)]() mutable -> result_type_t {
                           try {
                               result_type_t result = captured_future.get();
                               if (result.is_error()) {
                                   return captured_function(result.error());
                               }
                               return result;
                           } catch (...) {
                               std::fprintf(stderr, "[async_result] recover: threw exception\n");
                               return result_type_t(detail::make_async_exception_context("async recover: threw"));
                           }
                       }));
    }

    template <typename Function>
    auto make_async(Function&& function) -> async_result_t<result_value_t<std::invoke_result_t<Function>>, result_lean_v<std::invoke_result_t<Function>>> {
        using invoke_result_t = std::invoke_result_t<Function>;
        using value_t = result_value_t<invoke_result_t>;
        constexpr bool lean = result_lean_v<invoke_result_t>;

        return async_result_t<value_t, lean>(
            std::async(std::launch::async, [captured_function = std::forward<Function>(function)]() mutable -> invoke_result_t {
                try {
                    return captured_function();
                } catch (...) {
                    std::fprintf(stderr, "[async_result] make_async: threw exception\n");
                    return invoke_result_t(detail::make_async_exception_context("async make_async: function threw"));
                }
            }));
    }

}  // namespace error_system::async
