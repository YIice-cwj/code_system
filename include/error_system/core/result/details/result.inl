#pragma once

#include "error_system/core/error_context.h"
#include "error_system/core/i_error_notifier.h"
#include "error_system/utils/log.h"

/**
 * @file result.inl
 * @brief result_t 模板实现
 * @details result_t<T, Lean> 主模板与 result_t<void, Lean> 特化的成员函数实现。
 *          采用 union + result_state_t 手写判别式替代 std::variant，以支持 Move-Only 的
 *          error_context_t（C++17 std::variant 要求元素可拷贝）。union 成员通过 placement new
 *          构造、显式析构调用销毁，由 state_ 字段标记当前活跃成员。
 *          Lean=true 时错误路径仅存储 error_code_t，省去 error_context_t 的
 *          message/payload/cause/stack 开销，适用于热路径性能敏感场景。
 *
 *          小型函数（is_error/is_success/operator bool/value_pointer/value_or/operator->
 *          /make_success）已移至 .h 内联定义。
 *
 *          Debug 诊断：所有构造函数通过 loc 参数捕获创建点位置存入 created_at_，
 *          析构时若错误未被检查则输出 created_at_ 指向的用户代码位置（Release 零开销）。
 * @version 4.5.0
 * @date 2026-07-18
 */

/**
 * @brief main 模板的实现
 * @details 主模板成员函数的实现，按 .h 声明顺序组织。
 */
namespace error_system::core {

    namespace detail {
        /**
         * @brief Lean 模式条件下的错误存储类型
         * @details Lean=true 为 error_code_t（8B），Lean=false 为 error_context_t
         */
        template <bool Lean>
        using error_storage_for_t = std::conditional_t<Lean, error_code_t, error_context_t>;

        /**
         * @brief 构造错误存储并通知（const 消息版本）
         * @details Lean 路径仅存储 code 并走 on_code 通知；Full 路径构造 error_context_t 并走 on_error 通知。
         *          主模板与 void 特化的 make_error 共用此组助手以消除重复逻辑。
         * @param code 错误码
         * @param message 错误消息
         * @param location 源位置
         * @return 错误存储（Lean 返回 error_code_t，Full 返回 error_context_t）
         */
        template <bool Lean>
        inline error_storage_for_t<Lean> make_error_storage(error_code_t code, const std::string& message,
                                                             utils::source_location_t location) noexcept {
            if constexpr (Lean) {
                (void)message;
                (void)location;
                i_error_notifier_t::try_notify(code);
                return code;
            } else {
                error_context_t ctx{located_code_t{code, location}, message};
                i_error_notifier_t::try_notify(ctx);
                return ctx;
            }
        }

        /**
         * @brief 构造错误存储并通知（移动消息版本）
         * @param code 错误码
         * @param message 错误消息（右值，将被移动）
         * @param location 源位置
         * @return 错误存储
         */
        template <bool Lean>
        inline error_storage_for_t<Lean> make_error_storage(error_code_t code, std::string&& message,
                                                             utils::source_location_t location) noexcept {
            if constexpr (Lean) {
                (void)message;
                (void)location;
                i_error_notifier_t::try_notify(code);
                return code;
            } else {
                error_context_t ctx{located_code_t{code, location}, std::move(message)};
                i_error_notifier_t::try_notify(ctx);
                return ctx;
            }
        }

        /**
         * @brief 构造错误存储并通知（const error_context_t 版本）
         * @details Lean 路径提取 code 走 on_code 通知；Full 路径克隆上下文
         * @param context 错误上下文 const 引用
         * @return 错误存储
         */
        template <bool Lean>
        inline error_storage_for_t<Lean> make_error_storage(const error_context_t& context) noexcept {
            if constexpr (Lean) {
                i_error_notifier_t::try_notify(context.get_code());
                return context.get_code();
            } else {
                return context.clone();
            }
        }

        /**
         * @brief 构造错误存储并通知（error_context_t 右值版本）
         * @details Lean 路径提取 code 走 on_code 通知；Full 路径移动上下文
         * @param context 错误上下文右值
         * @return 错误存储
         */
        template <bool Lean>
        inline error_storage_for_t<Lean> make_error_storage(error_context_t&& context) noexcept {
            if constexpr (Lean) {
                i_error_notifier_t::try_notify(context.get_code());
                return context.get_code();
            } else {
                return std::move(context);
            }
        }
    }  // namespace detail

    /**
     * @brief 析构当前活跃的 union 成员
     * @details 根据 state_ 调用对应成员的析构函数，empty 状态无操作。
     *          析构后将 state_ 置为 empty。
     */
    template <typename T, bool Lean>
    void result_t<T, Lean>::destroy_active_() noexcept {
        switch (state_) {
            case result_state_t::value:
                storage_.value.~value_type_t();
                break;
            case result_state_t::error:
                storage_.error.~error_storage_t();
                break;
            case result_state_t::empty:
                break;
        }
        state_ = result_state_t::empty;
    }

    /**
     * @brief 析构前检查未消费错误
     * @details Debug 模式下若错误未被检查则输出诊断信息（指向 result_t 创建点位置）；
     *          Release 模式整段消除。
     */
    template <typename T, bool Lean>
    void result_t<T, Lean>::check_on_destroy_() const noexcept {
#ifndef NDEBUG
        if (!checked_ && state_ == result_state_t::error) {
            detail::report_unchecked_result(created_at_);
        }
#else
        (void)0;
#endif
    }

    /**
     * @brief 标记结果已被检查
     * @details Debug 下设置 checked_，Release 下空操作（编译器内联消除）
     */
    template <typename T, bool Lean>
    inline void result_t<T, Lean>::mark_checked_() const noexcept {
#ifndef NDEBUG
        checked_ = true;
#else
        (void)0;
#endif
    }

    /**
     * @brief Lean 模式专用私有构造函数
     * @details 直接接受 error_code_t，零堆开销。
     * @param code 错误码
     * @param loc 创建点位置（Debug 用于未检查诊断）
     */
    template <typename T, bool Lean>
    template <bool IsLean, typename>
    result_t<T, Lean>::result_t(error_code_t code, [[maybe_unused]] utils::source_location_t loc) noexcept
        : state_(result_state_t::error)
#ifndef NDEBUG
        , created_at_(loc)
#endif
    {
        new (&storage_.error) error_storage_t(code);
    }

    /**
     * @brief 从成功值构造（左值版本）
     * @param value 成功值
     * @param loc 创建点位置（Debug 用于未检查诊断）
     */
    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(const value_type_t& value, [[maybe_unused]] utils::source_location_t loc) noexcept(std::is_nothrow_copy_constructible_v<value_type_t>)
        : state_(result_state_t::value)
#ifndef NDEBUG
        , created_at_(loc)
#endif
    {
        new (&storage_.value) value_type_t(value);
    }

    /**
     * @brief 从成功值构造（右值版本）
     * @param value 成功值
     * @param loc 创建点位置（Debug 用于未检查诊断）
     */
    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(value_type_t&& value, [[maybe_unused]] utils::source_location_t loc) noexcept(std::is_nothrow_move_constructible_v<value_type_t>)
        : state_(result_state_t::value)
#ifndef NDEBUG
        , created_at_(loc)
#endif
    {
        new (&storage_.value) value_type_t(std::move(value));
    }

    /**
     * @brief 从错误上下文构造
     * @param context 错误上下文
     * @param loc 创建点位置（Debug 用于未检查诊断）
     */
    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(error_context_t context, [[maybe_unused]] utils::source_location_t loc) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>)
        : state_(result_state_t::error)
#ifndef NDEBUG
        , created_at_(loc)
#endif
    {
        if constexpr (Lean) {
            new (&storage_.error) error_storage_t(context.get_code());
        } else {
            new (&storage_.error) error_storage_t(std::move(context));
        }
    }

    /**
     * @brief 移动构造
     * @param other 另一个结果对象
     */
    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(result_t&& other) noexcept(std::is_nothrow_move_constructible_v<value_type_t>
                                                            && std::is_nothrow_move_constructible_v<error_storage_t>)
        : state_(other.state_)
#ifndef NDEBUG
        , checked_(other.checked_), created_at_(other.created_at_)
#endif
    {
        switch (state_) {
            case result_state_t::value:
                new (&storage_.value) value_type_t(std::move(other.storage_.value));
                break;
            case result_state_t::error:
                new (&storage_.error) error_storage_t(std::move(other.storage_.error));
                break;
            case result_state_t::empty:
                break;
        }
        other.destroy_active_();
        other.mark_checked_();
    }

    /**
     * @brief 移动赋值
     * @param other 另一个结果对象
     * @return 自身引用
     */
    template <typename T, bool Lean>
    result_t<T, Lean>& result_t<T, Lean>::operator=(result_t&& other) noexcept(std::is_nothrow_move_assignable_v<value_type_t>
                                                                               && std::is_nothrow_move_assignable_v<error_storage_t>) {
        if (this != &other) {
            check_on_destroy_();
            destroy_active_();
            state_ = other.state_;
#ifndef NDEBUG
            checked_ = other.checked_;
            created_at_ = other.created_at_;
#endif
            switch (state_) {
                case result_state_t::value:
                    new (&storage_.value) value_type_t(std::move(other.storage_.value));
                    break;
                case result_state_t::error:
                    new (&storage_.error) error_storage_t(std::move(other.storage_.error));
                    break;
                case result_state_t::empty:
                    break;
            }
            other.destroy_active_();
            other.mark_checked_();
        }
        return *this;
    }

    /**
     * @brief 析构函数
     * @details 检查未消费错误后析构活跃 union 成员
     */
    template <typename T, bool Lean>
    result_t<T, Lean>::~result_t() noexcept {
        check_on_destroy_();
        destroy_active_();
    }

    /**
     * @brief 错误构造工厂（推荐替代直接构造错误结果，避免重载混淆）
     * @details 委托 detail::make_error_storage 完成通知与存储构造，消除主模板与 void 特化的重复逻辑。
     *          location 同时作为 error_context_t 内部位置与 result_t::created_at_。
     * @param code 错误码
     * @param message 错误消息
     * @param location 源位置
     * @return 构造好的错误结果
     */
    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_error(error_code_t code, const std::string& message,
                                                     utils::source_location_t location) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>) {
        assert(!code.is_success_code() && "make_error called with a success code");
        return result_t(detail::make_error_storage<Lean>(code, message, location), location);
    }

    /**
     * @brief 错误构造工厂（移动消息版本）
     * @param code 错误码
     * @param message 错误消息（右值，将被移动）
     * @param location 源位置
     * @return 构造好的错误结果
     */
    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_error(error_code_t code, std::string&& message,
                                                     utils::source_location_t location) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>) {
        assert(!code.is_success_code() && "make_error called with a success code");
        return result_t(detail::make_error_storage<Lean>(code, std::move(message), location), location);
    }

    /**
     * @brief 错误构造工厂（从 const error_context_t，完整模式下内部克隆）
     * @param context 错误上下文 const 引用
     * @param loc 创建点位置
     * @return 构造好的错误结果
     */
    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_error(const error_context_t& context,
                                                     utils::source_location_t loc) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>) {
        return result_t(detail::make_error_storage<Lean>(context), loc);
    }

    /**
     * @brief 错误构造工厂（从 error_context_t 右值，无克隆开销）
     * @param context 错误上下文右值
     * @param loc 创建点位置
     * @return 构造好的错误结果
     */
    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_error(error_context_t&& context,
                                                     utils::source_location_t loc) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>) {
        return result_t(detail::make_error_storage<Lean>(std::move(context)), loc);
    }

    /**
     * @brief 获取成功值
     * @details 错误状态返回线程局部哨兵值（要求 T 可默认构造，否则用 value_pointer()）
     * @return 成功值 const 引用
     */
    template <typename T, bool Lean>
    const T& result_t<T, Lean>::value() const noexcept {
        static_assert(std::is_default_constructible_v<value_type_t>,
                      "result_t::value() requires T to be default-constructible. "
                      "Use value_pointer() for non-default-constructible types.");
        mark_checked_();
        assert(state_ == result_state_t::value && "result_t::value() called on an error result");
        if (state_ == result_state_t::value) {
            return storage_.value;
        }
        static thread_local const value_type_t sentinel{};
        return sentinel;
    }

    /**
     * @brief 获取成功值（可变版本）
     * @details 错误状态返回线程局部哨兵值（要求 T 可默认构造，否则用 value_pointer()）
     * @return 成功值引用
     */
    template <typename T, bool Lean>
    T& result_t<T, Lean>::value() noexcept {
        static_assert(std::is_default_constructible_v<value_type_t>,
                      "result_t::value() requires T to be default-constructible. "
                      "Use value_pointer() for non-default-constructible types.");
        mark_checked_();
        assert(state_ == result_state_t::value && "result_t::value() called on an error result");
        if (state_ == result_state_t::value) {
            return storage_.value;
        }
        static thread_local value_type_t sentinel{};
        return sentinel;
    }

    /**
     * @brief 获取错误上下文
     * @details 完整模式返回 const 引用（成功状态返回线程局部哨兵值）；
     *          Lean 模式返回值类型 error_context_t（临时构造仅含 code，无 file:line）
     * @return 错误上下文
     */
    template <typename T, bool Lean>
    auto result_t<T, Lean>::error() const noexcept
        -> std::conditional_t<Lean, error_context_t, const error_context_t&> {
        mark_checked_();
        assert(state_ == result_state_t::error && "result_t::error() called on a success result");
        if constexpr (Lean) {
            if (state_ == result_state_t::error) {
                return error_context_t::make_minimal(storage_.error);
            }
            return error_context_t{};
        } else {
            if (state_ == result_state_t::error) {
                return storage_.error;
            }
            static thread_local const error_context_t sentinel{};
            return sentinel;
        }
    }

    /**
     * @brief 获取错误上下文可变引用
     * @details 仅完整模式可用，Lean 模式 SFINAE 禁用
     * @return 错误上下文可变引用
     */
    template <typename T, bool Lean>
    template <bool IsLean, typename>
    error_context_t& result_t<T, Lean>::error() noexcept {
        mark_checked_();
        assert(state_ == result_state_t::error && "result_t::error() called on a success result");
        if (state_ == result_state_t::error) {
            return storage_.error;
        }
        static thread_local error_context_t sentinel{};
        return sentinel;
    }

    /**
     * @brief 获取错误码
     * @details Lean 模式直接返回存储值，完整模式从 error_context_t 提取
     * @return 错误码
     */
    template <typename T, bool Lean>
    error_code_t result_t<T, Lean>::error_code() const noexcept {
        mark_checked_();
        if (state_ == result_state_t::error) {
            if constexpr (Lean) {
                return storage_.error;
            } else {
                return storage_.error.get_code();
            }
        }
        return error_code_t::make_success();
    }

    /**
     * @brief 对成功值进行映射转换（const 左值版本）
     * @tparam Function 可调用对象类型，签名为 U(const value_type_t&)
     * @param function 处理成功值的可调用对象
     * @return 转换后的新 result_t
     */
    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::map(Function&& function) const& noexcept
        -> result_t<decltype(std::invoke(std::forward<Function>(function), std::declval<const value_type_t&>())), Lean> {
        using new_type = decltype(std::invoke(std::forward<Function>(function), std::declval<const value_type_t&>()));
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<new_type, Lean>(storage_.error);
            } else {
                return result_t<new_type, Lean>(error().clone());
            }
        }
        try {
            return result_t<new_type, Lean>(std::invoke(std::forward<Function>(function), value()));
        } catch (...) {
            LOG_ERROR("[result_t] map: std::invoke threw exception");
            return result_t<new_type, Lean>(detail::make_invoke_exception_context("map: function threw exception"));
        }
    }

    /**
     * @brief 对成功值进行映射转换（右值版本）
     * @tparam Function 可调用对象类型，签名为 U(value_type_t&&)
     * @param function 处理成功值的可调用对象
     * @return 转换后的新 result_t
     */
    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::map(Function&& function) && noexcept
        -> result_t<decltype(std::invoke(std::forward<Function>(function), std::move(value()))), Lean> {
        using new_type = decltype(std::invoke(std::forward<Function>(function), std::move(value())));
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<new_type, Lean>(storage_.error);
            } else {
                return result_t<new_type, Lean>(std::move(error()));
            }
        }
        try {
            return result_t<new_type, Lean>(std::invoke(std::forward<Function>(function), std::move(value())));
        } catch (...) {
            LOG_ERROR("[result_t] map(&&): std::invoke threw exception");
            return result_t<new_type, Lean>(detail::make_invoke_exception_context("map(&&): function threw exception"));
        }
    }

    /**
     * @brief 对错误上下文进行映射转换（const 左值版本）
     * @tparam Function 可调用对象类型，签名为 error_context_t(const error_context_t&)
     * @param function 处理错误上下文的可调用对象
     * @return 转换后的新 result_t
     */
    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::map_error(Function&& function) const& noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(storage_.error);
            } else {
                try {
                    return result_t<T, Lean>(std::invoke(std::forward<Function>(function), error()));
                } catch (...) {
                    LOG_ERROR("[result_t] map_error: std::invoke threw exception");
                    return result_t<T, Lean>(error().clone());
                }
            }
        }
        return result_t<T, Lean>(value());
    }

    /**
     * @brief 对错误上下文进行映射转换（右值版本）
     * @tparam Function 可调用对象类型，签名为 error_context_t(error_context_t&&)
     * @param function 处理错误上下文的可调用对象
     * @return 转换后的新 result_t
     */
    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::map_error(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(storage_.error);
            } else {
                try {
                    return result_t<T, Lean>(std::invoke(std::forward<Function>(function), std::move(error())));
                } catch (...) {
                    LOG_ERROR("[result_t] map_error(&&): std::invoke threw exception");
                    return result_t<T, Lean>(detail::make_invoke_exception_context("map_error(&&): function threw exception"));
                }
            }
        }
        return std::move(*this);
    }

    /**
     * @brief 链式操作：成功时调用 function 处理值（右值版本，移动语义）
     * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
     * @param function 处理成功值的可调用对象
     * @return function 返回的结果
     */
    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::and_then(Function&& function) && noexcept
        -> decltype(std::invoke(std::forward<Function>(function), std::move(value()))) {
        using return_type = decltype(std::invoke(std::forward<Function>(function), std::move(value())));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(storage_.error);
            } else {
                return return_type(std::move(error()));
            }
        }
        try {
            return std::invoke(std::forward<Function>(function), std::move(value()));
        } catch (...) {
            LOG_ERROR("[result_t] and_then(&&): std::invoke threw exception");
            return return_type(detail::make_invoke_exception_context("and_then(&&): function threw exception"));
        }
    }

    /**
     * @brief 链式操作：成功时调用 function 处理值（左值版本，错误时克隆保持 *this 不变）
     * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
     * @param function 处理成功值的可调用对象
     * @return function 返回的结果
     */
    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::and_then(Function&& function) & noexcept
        -> decltype(std::invoke(std::forward<Function>(function), value())) {
        using return_type = decltype(std::invoke(std::forward<Function>(function), value()));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(storage_.error);
            } else {
                return return_type(error().clone());
            }
        }
        try {
            return std::invoke(std::forward<Function>(function), value());
        } catch (...) {
            LOG_ERROR("[result_t] and_then(&): std::invoke threw exception");
            return return_type(detail::make_invoke_exception_context("and_then(&): function threw exception"));
        }
    }

    /**
     * @brief 链式操作：成功时调用 function 处理值（const 左值版本，错误时克隆）
     * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
     * @param function 处理成功值的可调用对象
     * @return function 返回的结果
     */
    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::and_then(Function&& function) const& noexcept
        -> decltype(std::invoke(std::forward<Function>(function), value())) {
        using return_type = decltype(std::invoke(std::forward<Function>(function), value()));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(storage_.error);
            } else {
                return return_type(error().clone());
            }
        }
        try {
            return std::invoke(std::forward<Function>(function), value());
        } catch (...) {
            LOG_ERROR("[result_t] and_then(const&): std::invoke threw exception");
            return return_type(detail::make_invoke_exception_context("and_then(const&): function threw exception"));
        }
    }

    /**
     * @brief 错误时链式操作：错误时调用 function 处理错误（右值版本，移动语义）
     * @tparam Function 可调用对象类型，签名为 result_t(error_context_t&&)
     * @param function 处理错误的可调用对象
     * @return 成功时移动自身，错误时返回 function 处理后的结果
     */
    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::or_else(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(storage_.error);
            } else {
                try {
                    return std::invoke(std::forward<Function>(function), std::move(error()));
                } catch (...) {
                    LOG_ERROR("[result_t] or_else(&&): std::invoke threw exception");
                    return result_t<T, Lean>(detail::make_invoke_exception_context("or_else(&&): function threw exception"));
                }
            }
        }
        return std::move(*this);
    }

    /**
     * @brief 错误时链式操作：错误时调用 function 处理错误（左值版本，成功时克隆）
     * @tparam Function 可调用对象类型，签名为 result_t(const error_context_t&)
     * @param function 处理错误的可调用对象
     * @return 成功时克隆自身，错误时返回 function 处理后的结果
     */
    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::or_else(Function&& function) & noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(storage_.error);
            } else {
                try {
                    return std::invoke(std::forward<Function>(function), error());
                } catch (...) {
                    LOG_ERROR("[result_t] or_else(&): std::invoke threw exception");
                    return result_t<T, Lean>(error().clone());
                }
            }
        }
        return result_t<T, Lean>(value());
    }

    /**
     * @brief 模式匹配：成功调用 success_function，错误调用 error_function
     * @details 两者须返回相同类型
     * @tparam SuccessFunction 成功路径可调用对象类型
     * @tparam ErrorFunction 错误路径可调用对象类型
     * @param success_function 成功时调用的可调用对象
     * @param error_function 错误时调用的可调用对象
     * @return 两个 function 返回类型的共同类型
     */
    template <typename T, bool Lean>
    template <typename SuccessFunction, typename ErrorFunction>
    auto result_t<T, Lean>::match(SuccessFunction&& success_function, ErrorFunction&& error_function) const
        noexcept(std::is_nothrow_invocable_v<SuccessFunction&, const value_type_t&>
                 && std::is_nothrow_invocable_v<ErrorFunction&, const error_context_t&>)
        -> decltype(success_function(std::declval<const value_type_t&>())) {
        mark_checked_();
        if (state_ == result_state_t::value) {
            return success_function(storage_.value);
        }
        if (state_ == result_state_t::error) {
            if constexpr (Lean) {
                return error_function(error_context_t::make_minimal(storage_.error));
            } else {
                return error_function(storage_.error);
            }
        }
        return {};
    }

    /**
     * @brief 传播时附加 payload 上下文（左值版本）
     * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
     * @tparam Key 键类型
     * @tparam V 值类型
     * @param key 字段名
     * @param value 字段值
     * @return 自身引用（支持链式调用）
     */
    template <typename T, bool Lean>
    template <typename Key, typename V, bool IsLean, typename>
    result_t<T, Lean>& result_t<T, Lean>::context(Key&& key, V&& value) & noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                error().with(std::forward<Key>(key), std::forward<V>(value));
            }
        }
        return *this;
    }

    /**
     * @brief 传播时附加 payload 上下文（右值版本，返回移动后的新对象）
     * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
     * @tparam Key 键类型
     * @tparam V 值类型
     * @param key 字段名
     * @param value 字段值
     * @return 移动后的新结果对象
     */
    template <typename T, bool Lean>
    template <typename Key, typename V, bool IsLean, typename>
    result_t<T, Lean> result_t<T, Lean>::context(Key&& key, V&& value) && noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                error().with(std::forward<Key>(key), std::forward<V>(value));
            }
        }
        return std::move(*this);
    }

    /**
     * @brief 转换为字符串描述
     * @details 成功返回 "[OK: <value>]"（若 T 可被 string_format_t 格式化），
     *          错误返回 "[ERR: <context>]"（Full 模式委托 error_context_t::to_string，
     *          Lean 模式输出 [ERR: <raw_code>]），empty 返回 "[empty]"。
     *          提供本方法后 result_t 可直接作为 string_format_t 的 "{}" 参数。
     * @return 状态描述字符串
     *
     * 实现思路：按 state_ 分支处理，成功路径用 if constexpr 判定 T 是否可被
     *          string_format_t 格式化，避免对任意 T 强求 to_string 支持。
     *          bad_alloc 时回退到固定字符串，保证 noexcept。
     */
    template <typename T, bool Lean>
    std::string result_t<T, Lean>::to_string() const noexcept {
        mark_checked_();
        try {
            switch (state_) {
                case result_state_t::value: {
                    if constexpr (utils::is_member_to_string_v<value_type_t> ||
                                  utils::is_global_to_string_v<value_type_t> ||
                                  std::is_arithmetic_v<value_type_t> ||
                                  std::is_convertible_v<value_type_t, std::string_view>) {
                        return utils::string_format_t::format("[OK: {}]", storage_.value);
                    } else {
                        return "[OK: <value>]";
                    }
                }
                case result_state_t::error: {
                    if constexpr (Lean) {
                        return utils::string_format_t::format("[ERR: {}]", storage_.error.get_code());
                    } else {
                        return "[ERR: " + storage_.error.to_string() + "]";
                    }
                }
                case result_state_t::empty:
                default:
                    return "[empty]";
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[result_t] to_string: bad_alloc");
            return "[error: alloc failed]";
        }
    }

    /**
     * @brief result_t<void, Lean> 特化实现
     * @details void 特化的成员函数实现，按 .h 声明顺序组织。
     */

    /**
     * @brief 析构当前活跃的 union 成员
     * @details error 状态调用 error_storage_t 析构，其他状态无操作。
     *          析构后将 state_ 置为 empty。
     */
    template <bool Lean>
    inline void result_t<void, Lean>::destroy_active_() noexcept {
        if (state_ == result_state_t::error) {
            storage_.error.~error_storage_t();
        }
        state_ = result_state_t::empty;
    }

    /**
     * @brief 析构前检查未消费错误
     * @details Debug 模式下若错误未被检查则输出诊断信息（指向 result_t 创建点位置）；
     *          Release 模式整段消除。
     */
    template <bool Lean>
    inline void result_t<void, Lean>::check_on_destroy_() const noexcept {
#ifndef NDEBUG
        if (!checked_ && state_ == result_state_t::error) {
            detail::report_unchecked_result(created_at_);
        }
#else
        (void)0;
#endif
    }

    /**
     * @brief 标记结果已被检查
     * @details Debug 下设置 checked_，Release 下空操作（编译器内联消除）
     */
    template <bool Lean>
    inline void result_t<void, Lean>::mark_checked_() const noexcept {
#ifndef NDEBUG
        checked_ = true;
#else
        (void)0;
#endif
    }

    /**
     * @brief Lean 模式专用私有构造函数
     * @details 直接接受 error_code_t，零堆开销。
     * @param code 错误码
     * @param loc 创建点位置（Debug 用于未检查诊断）
     */
    template <bool Lean>
    template <bool IsLean, typename>
    inline result_t<void, Lean>::result_t(error_code_t code, [[maybe_unused]] utils::source_location_t loc) noexcept
        : state_(result_state_t::error)
#ifndef NDEBUG
        , created_at_(loc)
#endif
    {
        new (&storage_.error) error_storage_t(code);
    }

    /**
     * @brief 默认构造（成功状态）
     * @param loc 创建点位置（Debug 用于未检查诊断）
     * @return 无返回值（构造函数）
     */
    template <bool Lean>
    inline result_t<void, Lean>::result_t([[maybe_unused]] utils::source_location_t loc) noexcept
        : state_(result_state_t::empty)
#ifndef NDEBUG
        , created_at_(loc)
#endif
    {}

    /**
     * @brief 从错误上下文构造
     * @param context 错误上下文
     * @param loc 创建点位置（Debug 用于未检查诊断）
     */
    template <bool Lean>
    inline result_t<void, Lean>::result_t(error_context_t context, [[maybe_unused]] utils::source_location_t loc) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>)
        : state_(result_state_t::error)
#ifndef NDEBUG
        , created_at_(loc)
#endif
    {
        if constexpr (Lean) {
            new (&storage_.error) error_storage_t(context.get_code());
        } else {
            new (&storage_.error) error_storage_t(std::move(context));
        }
    }

    /**
     * @brief 移动构造
     * @param other 另一个结果对象
     */
    template <bool Lean>
    inline result_t<void, Lean>::result_t(result_t&& other) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>)
        : state_(other.state_)
#ifndef NDEBUG
        , checked_(other.checked_), created_at_(other.created_at_)
#endif
    {
        if (state_ == result_state_t::error) {
            new (&storage_.error) error_storage_t(std::move(other.storage_.error));
        }
        other.destroy_active_();
        other.mark_checked_();
    }

    /**
     * @brief 移动赋值
     * @param other 另一个结果对象
     * @return 自身引用
     */
    template <bool Lean>
    inline result_t<void, Lean>& result_t<void, Lean>::operator=(result_t&& other) noexcept(std::is_nothrow_move_assignable_v<error_storage_t>) {
        if (this != &other) {
            check_on_destroy_();
            destroy_active_();
            state_ = other.state_;
#ifndef NDEBUG
            checked_ = other.checked_;
            created_at_ = other.created_at_;
#endif
            if (state_ == result_state_t::error) {
                new (&storage_.error) error_storage_t(std::move(other.storage_.error));
            }
            other.destroy_active_();
            other.mark_checked_();
        }
        return *this;
    }

    /**
     * @brief 析构函数
     * @details 检查未消费错误后析构活跃 union 成员
     */
    template <bool Lean>
    inline result_t<void, Lean>::~result_t() noexcept {
        check_on_destroy_();
        destroy_active_();
    }

    /**
     * @brief 错误构造工厂
     * @details 委托 detail::make_error_storage 完成通知与存储构造，与主模板共用同一组助手。
     *          location 同时作为 error_context_t 内部位置与 result_t::created_at_。
     * @param code 错误码
     * @param message 错误消息
     * @param location 源位置
     * @return 构造好的错误结果
     */
    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_error(error_code_t code, const std::string& message,
                                                                  utils::source_location_t location) noexcept {
        assert(!code.is_success_code() && "make_error called with a success code");
        return result_t<void, Lean>(detail::make_error_storage<Lean>(code, message, location), location);
    }

    /**
     * @brief 错误构造工厂（移动消息版本）
     * @param code 错误码
     * @param message 错误消息（右值，将被移动）
     * @param location 源位置
     * @return 构造好的错误结果
     */
    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_error(error_code_t code, std::string&& message,
                                                                  utils::source_location_t location) noexcept {
        assert(!code.is_success_code() && "make_error called with a success code");
        return result_t<void, Lean>(detail::make_error_storage<Lean>(code, std::move(message), location), location);
    }

    /**
     * @brief 错误构造工厂（从 const error_context_t，完整模式下内部克隆）
     * @param context 错误上下文 const 引用
     * @param loc 创建点位置
     * @return 构造好的错误结果
     */
    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_error(const error_context_t& context,
                                                                  utils::source_location_t loc) noexcept {
        return result_t<void, Lean>(detail::make_error_storage<Lean>(context), loc);
    }

    /**
     * @brief 错误构造工厂（从 error_context_t 右值，无克隆开销）
     * @param context 错误上下文右值
     * @param loc 创建点位置
     * @return 构造好的错误结果
     */
    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_error(error_context_t&& context,
                                                                  utils::source_location_t loc) noexcept {
        return result_t<void, Lean>(detail::make_error_storage<Lean>(std::move(context)), loc);
    }

    /**
     * @brief 获取错误上下文
     * @details 完整模式返回 const 引用（成功状态返回线程局部哨兵值）；
     *          Lean 模式返回值类型 error_context_t（临时构造仅含 code，无 file:line）
     * @return 错误上下文
     */
    template <bool Lean>
    inline auto result_t<void, Lean>::error() const noexcept
        -> std::conditional_t<Lean, error_context_t, const error_context_t&> {
        mark_checked_();
        assert(state_ == result_state_t::error && "result_t<void>::error() called on a success result");
        if constexpr (Lean) {
            if (state_ == result_state_t::error) {
                return error_context_t::make_minimal(storage_.error);
            }
            return error_context_t{};
        } else {
            if (state_ == result_state_t::error) {
                return storage_.error;
            }
            static thread_local const error_context_t sentinel{};
            return sentinel;
        }
    }

    /**
     * @brief 获取错误上下文可变引用
     * @details 仅完整模式可用，Lean 模式 SFINAE 禁用
     * @return 错误上下文可变引用
     */
    template <bool Lean>
    template <bool IsLean, typename>
    inline error_context_t& result_t<void, Lean>::error() noexcept {
        mark_checked_();
        assert(state_ == result_state_t::error && "result_t<void>::error() called on a success result");
        if (state_ == result_state_t::error) {
            return storage_.error;
        }
        static thread_local error_context_t sentinel{};
        return sentinel;
    }

    /**
     * @brief 获取错误码
     * @details Lean 模式直接返回存储值，完整模式从 error_context_t 提取
     * @return 错误码
     */
    template <bool Lean>
    inline error_code_t result_t<void, Lean>::error_code() const noexcept {
        mark_checked_();
        if (state_ == result_state_t::error) {
            if constexpr (Lean) {
                return storage_.error;
            } else {
                return storage_.error.get_code();
            }
        }
        return error_code_t::make_success();
    }

    /**
     * @brief 链式操作：成功时调用 function 处理（右值版本，移动语义）
     * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
     * @param function 处理成功路径的可调用对象
     * @return function 返回的结果
     */
    template <bool Lean>
    template <typename Function>
    auto result_t<void, Lean>::and_then(Function&& function) && noexcept
        -> decltype(std::invoke(std::forward<Function>(function))) {
        using return_type = decltype(std::invoke(std::forward<Function>(function)));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(storage_.error);
            } else {
                return return_type(std::move(error()));
            }
        }
        try {
            return std::invoke(std::forward<Function>(function));
        } catch (...) {
            LOG_ERROR("[result_t<void>] and_then(&&): std::invoke threw exception");
            return return_type(detail::make_invoke_exception_context("and_then(&&): function threw exception"));
        }
    }

    /**
     * @brief 链式操作：成功时调用 function 处理（左值版本，错误时克隆）
     * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
     * @param function 处理成功路径的可调用对象
     * @return function 返回的结果
     */
    template <bool Lean>
    template <typename Function>
    auto result_t<void, Lean>::and_then(Function&& function) & noexcept
        -> decltype(std::invoke(std::forward<Function>(function))) {
        using return_type = decltype(std::invoke(std::forward<Function>(function)));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(storage_.error);
            } else {
                return return_type(error().clone());
            }
        }
        try {
            return std::invoke(std::forward<Function>(function));
        } catch (...) {
            LOG_ERROR("[result_t<void>] and_then(&): std::invoke threw exception");
            return return_type(detail::make_invoke_exception_context("and_then(&): function threw exception"));
        }
    }

    /**
     * @brief 对错误上下文进行映射转换（const 左值版本）
     * @tparam Function 可调用对象类型，签名为 error_context_t(const error_context_t&)
     * @param function 处理错误上下文的可调用对象
     * @return 转换后的新 result_t
     */
    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::map_error(Function&& function) const& noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(storage_.error);
            } else {
                try {
                    return result_t<void, Lean>(std::invoke(std::forward<Function>(function), error()));
                } catch (...) {
                    LOG_ERROR("[result_t<void>] map_error: std::invoke threw exception");
                    return result_t<void, Lean>(error().clone());
                }
            }
        }
        return result_t<void, Lean>();
    }

    /**
     * @brief 对错误上下文进行映射转换（右值版本）
     * @tparam Function 可调用对象类型，签名为 error_context_t(error_context_t&&)
     * @param function 处理错误上下文的可调用对象
     * @return 转换后的新 result_t
     */
    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::map_error(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(storage_.error);
            } else {
                try {
                    return result_t<void, Lean>(std::invoke(std::forward<Function>(function), std::move(error())));
                } catch (...) {
                    LOG_ERROR("[result_t<void>] map_error(&&): std::invoke threw exception");
                    return result_t<void, Lean>(detail::make_invoke_exception_context("map_error(&&): function threw exception"));
                }
            }
        }
        return std::move(*this);
    }

    /**
     * @brief 错误时链式操作：错误时调用 function 处理错误（右值版本，移动语义）
     * @tparam Function 可调用对象类型，签名为 result_t(error_context_t&&)
     * @param function 处理错误的可调用对象
     * @return 成功时移动自身，错误时返回 function 处理后的结果
     */
    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::or_else(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(storage_.error);
            } else {
                try {
                    return std::invoke(std::forward<Function>(function), std::move(error()));
                } catch (...) {
                    LOG_ERROR("[result_t<void>] or_else(&&): std::invoke threw exception");
                    return result_t<void, Lean>(detail::make_invoke_exception_context("or_else(&&): function threw exception"));
                }
            }
        }
        return std::move(*this);
    }

    /**
     * @brief 错误时链式操作：错误时调用 function 处理错误（左值版本，成功时克隆）
     * @tparam Function 可调用对象类型，签名为 result_t(const error_context_t&)
     * @param function 处理错误的可调用对象
     * @return 成功时克隆自身，错误时返回 function 处理后的结果
     */
    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::or_else(Function&& function) & noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(storage_.error);
            } else {
                try {
                    return std::invoke(std::forward<Function>(function), error());
                } catch (...) {
                    LOG_ERROR("[result_t<void>] or_else(&): std::invoke threw exception");
                    return result_t<void, Lean>(error().clone());
                }
            }
        }
        return result_t<void, Lean>();
    }

    /**
     * @brief 传播时附加 payload 上下文（左值版本）
     * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
     * @tparam Key 键类型
     * @tparam V 值类型
     * @param key 字段名
     * @param value 字段值
     * @return 自身引用（支持链式调用）
     */
    template <bool Lean>
    template <typename Key, typename V, bool IsLean, typename>
    result_t<void, Lean>& result_t<void, Lean>::context(Key&& key, V&& value) & noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                error().with(std::forward<Key>(key), std::forward<V>(value));
            }
        }
        return *this;
    }

    /**
     * @brief 传播时附加 payload 上下文（右值版本，返回移动后的新对象）
     * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
     * @tparam Key 键类型
     * @tparam V 值类型
     * @param key 字段名
     * @param value 字段值
     * @return 移动后的新结果对象
     */
    template <bool Lean>
    template <typename Key, typename V, bool IsLean, typename>
    result_t<void, Lean> result_t<void, Lean>::context(Key&& key, V&& value) && noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                error().with(std::forward<Key>(key), std::forward<V>(value));
            }
        }
        return std::move(*this);
    }

    /**
     * @brief 转换为字符串描述
     * @details 成功返回 "[OK]"，错误返回 "[ERR: <context>]"（Full 模式委托
     *          error_context_t::to_string，Lean 模式输出 raw code）。
     *          提供本方法后 result_t 可直接作为 string_format_t 的 "{}" 参数。
     * @return 状态描述字符串
     *
     * 实现思路：void 特化无成功值，成功路径直接返回 "[OK]"；
     *          错误路径与主模板一致，按 Lean/Full 分支处理。
     *          bad_alloc 时回退到固定字符串，保证 noexcept。
     */
    template <bool Lean>
    std::string result_t<void, Lean>::to_string() const noexcept {
        mark_checked_();
        try {
            if (state_ == result_state_t::error) {
                if constexpr (Lean) {
                    return utils::string_format_t::format("[ERR: {}]", storage_.error.get_code());
                } else {
                    return "[ERR: " + storage_.error.to_string() + "]";
                }
            }
            return "[OK]";
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[result_t<void>] to_string: bad_alloc");
            return "[error: alloc failed]";
        }
    }

}  // namespace error_system::core
