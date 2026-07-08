#pragma once
#include <future>
#include <type_traits>
#include <utility>

#include "error_system/core/result/result.h"

/**
 * @file async_result.h
 * @brief 异步结果链式适配器
 * @details 基于 std::future<result_t<T>> 的 then/recover 链式 API，
 *          为 C++17 提供 monadic 风格的异步错误处理能力。
 *          无三方库依赖，底层使用 std::async(std::launch::async) 调度。
 *
 *          升级路径：C++20 引入 std::future::then 与协程后，可平滑迁移至
 *          协程化的 async_result_t，无需修改调用方代码（API 兼容）。
 *
 *          语义：
 *          - then(Function): Function 接收 result_t<T, Lean>，返回 result_t<U, Lean>，始终调用
 *          - recover(Function): Function 接收 error_context_t，返回 result_t<T, Lean>，仅错误时调用
 *
 *          链式调用采用消费语义（rvalue-qualified），临时对象自动链式传递：
 * @code
 *   auto result = make_async([] { return result_t<int>::make_success(42); })
 *       .then([](result_t<int> result) { return result_t<int>::make_success(result.value() * 2); })
 *       .recover([](error_context_t) { return result_t<int>::make_success(0); })
 *       .get();
 * @endcode
 * @author yiice
 * @version 3.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */
namespace error_system::async {

    /**
     * @brief 从 result_t<T, Lean> 提取 T 与 Lean 的类型萃取
     * @details 用于 then 回调返回值类型推导与 make_async 的 Lean 参数推导。
     */
    template <typename ResultType>
    struct result_traits;
    /**
     * @brief result_t<T, Lean> 的特化版本
     * @tparam T 结果值类型
     * @tparam Lean 精简模式标志
     */
    template <typename T, bool Lean>
    struct result_traits<core::result_t<T, Lean>> {
        using value_type = T;
        static constexpr bool lean = Lean;
    };
    /**
     * @brief 从 result_t 类型提取值类型
     * @tparam ResultType result_t<T, Lean> 类型
     */
    template <typename ResultType>
    using result_value_t = typename result_traits<std::decay_t<ResultType>>::value_type;
    /**
     * @brief 从 result_t 类型提取 Lean 标志
     * @tparam ResultType result_t<T, Lean> 类型
     */
    template <typename ResultType>
    constexpr bool result_lean_v = result_traits<std::decay_t<ResultType>>::lean;

    /**
     * @brief 异步结果链式适配器
     * @details 封装 std::future<result_t<T, Lean>>，提供 then/recover 链式 API。
     *          then/recover 为 rvalue-qualified（消费 *this），链式调用在临时对象上自动传递。
     *          回调异常被捕获并转换为 fatal 错误结果，保证 get() 永不抛出。
     * @tparam T 结果值类型
     * @tparam Lean 是否使用精简模式（与 result_t 的 Lean 参数语义一致）
     */
    template <typename T, bool Lean = false>
    class async_result_t {
    public:
        using value_type_t = T;
        using result_type_t = core::result_t<T, Lean>;

    private:
        std::future<result_type_t> future_;

    public:
        async_result_t() noexcept = default;
        explicit async_result_t(std::future<result_type_t> future) noexcept;
        explicit async_result_t(result_type_t result) noexcept;

        async_result_t(const async_result_t&) = delete;
        async_result_t& operator=(const async_result_t&) = delete;
        async_result_t(async_result_t&&) noexcept = default;
        async_result_t& operator=(async_result_t&&) noexcept = default;
        ~async_result_t() = default;

        /**
         * @brief 阻塞等待并获取结果
         * @return result_type_t 结果（永不抛出，内部异常已转换为 fatal 错误）
         */
        [[nodiscard]] result_type_t get();

        /**
         * @brief 阻塞等待结果就绪（不获取）
         */
        void wait() const noexcept;

        /**
         * @brief 检查结果是否就绪（非阻塞）
         * @return bool 就绪返回 true
         */
        [[nodiscard]] bool is_ready() const noexcept;

        /**
         * @brief 检查 future 是否有效（未被 get/move 走）
         * @return bool 有效返回 true
         */
        [[nodiscard]] bool valid() const noexcept;

        /**
         * @brief 链式转换（消费 *this）
         * @details Function 接收 result_t<T, Lean>，返回 result_t<U, Lean>。
         *          Function 始终被调用（无论成功/错误），由 Function 决定如何处理。
         *          Function 抛出的异常被捕获并转换为 fatal 错误结果。
         * @param function 转换函数
         * @return async_result_t<U, Lean> 新的异步结果
         */
        template <typename Function>
        [[nodiscard]] auto then(Function&& function) && -> async_result_t<result_value_t<std::invoke_result_t<Function, result_type_t>>, Lean>;

        /**
         * @brief 错误恢复（消费 *this）
         * @details Function 接收 error_context_t，返回 result_t<T, Lean>。
         *          仅当结果为错误时调用 Function；成功时直接传递原结果。
         *          Function 抛出的异常被捕获并转换为 fatal 错误结果。
         * @param function 恢复函数
         * @return async_result_t<T, Lean> 新的异步结果
         */
        template <typename Function>
        [[nodiscard]] async_result_t<T, Lean> recover(Function&& function) &&;
    };

    /**
     * @brief 从可调用对象创建异步结果
     * @details Function 返回 result_t<T, Lean>，包装为 async_result_t<T, Lean>。
     *          Lean 参数从 Function 的返回类型自动推导。
     *          底层使用 std::async(std::launch::async) 调度。
     * @param function 返回 result_t 的可调用对象
     * @return async_result_t<T, Lean> 异步结果
     */
    template <typename Function>
    [[nodiscard]] auto make_async(Function&& function) -> async_result_t<result_value_t<std::invoke_result_t<Function>>, result_lean_v<std::invoke_result_t<Function>>>;

}  // namespace error_system::async

#include "error_system/async/details/async_result.inl"
