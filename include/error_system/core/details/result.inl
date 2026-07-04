#pragma once

/**
 * @file result.inl
 * @brief result_t 模板实现
 * @details result_t<T, Lean> 主模板与 result_t<void, Lean> 特化的成员函数实现。
 *          Lean=true 时错误路径仅存储 error_code_t，省去 error_context_t 的
 *          message/payload/cause/stack 开销，适用于热路径性能敏感场景。
 * @author yiice
 * @version 2.4.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief result_t<T, Lean> 主模板实现
     */

    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(error_code_t code) noexcept(std::is_nothrow_move_constructible_v<error_code_t>)
        : value_or_error_(code) {}

    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_error(error_code_t code, const std::string& message,
                                                     utils::source_location_t location) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>) {
        if constexpr (Lean) {
            (void)message;
            (void)location;
            return result_t(code);
        } else {
            return result_t(error_context_t{located_code_t{code, location}, message});
        }
    }

    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_error(error_code_t code, std::string&& message,
                                                     utils::source_location_t location) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>) {
        if constexpr (Lean) {
            (void)message;
            (void)location;
            return result_t(code);
        } else {
            return result_t(error_context_t{located_code_t{code, location}, std::move(message)});
        }
    }

    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_error(const error_context_t& context) noexcept(std::is_nothrow_copy_constructible_v<error_storage_t>) {
        if constexpr (Lean) {
            return result_t(context.get_code());
        } else {
            return result_t(context);
        }
    }

    template <typename T, bool Lean>
    result_t<T, Lean> result_t<T, Lean>::make_success(value_type_t value) noexcept(std::is_nothrow_move_constructible_v<value_type_t>) {
        return result_t(std::move(value));
    }

    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(const value_type_t& value) noexcept(std::is_nothrow_copy_constructible_v<value_type_t>) : value_or_error_(value) {}

    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(value_type_t&& value) noexcept(std::is_nothrow_move_constructible_v<value_type_t>) : value_or_error_(std::move(value)) {}

    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(const error_context_t& error_context) noexcept(std::is_nothrow_copy_constructible_v<error_storage_t>)
        : value_or_error_([&] {
            if constexpr (Lean) {
                return error_storage_t{error_context.get_code()};
            } else {
                return error_storage_t{error_context};
            }
        }()) {}

    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(error_context_t&& error_context) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>)
        : value_or_error_([&] {
            if constexpr (Lean) {
                return error_storage_t{error_context.get_code()};
            } else {
                return error_storage_t{std::move(error_context)};
            }
        }()) {}

    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(const result_t& other) noexcept(std::is_nothrow_copy_constructible_v<value_type_t>
                                                                && std::is_nothrow_copy_constructible_v<error_storage_t>)
        : value_or_error_(other.value_or_error_), checked_(false) {}

    template <typename T, bool Lean>
    result_t<T, Lean>::result_t(result_t&& other) noexcept(std::is_nothrow_move_constructible_v<value_type_t>)
        : value_or_error_(std::move(other.value_or_error_)), checked_(other.checked_) {
        other.checked_ = true;
    }

    template <typename T, bool Lean>
    result_t<T, Lean>& result_t<T, Lean>::operator=(result_t&& other) noexcept(std::is_nothrow_move_assignable_v<value_type_t>) {
        if (this != &other) {
            check_on_destroy_();
            value_or_error_ = std::move(other.value_or_error_);
            checked_ = other.checked_;
            other.checked_ = true;
        }
        return *this;
    }

    template <typename T, bool Lean>
    result_t<T, Lean>::~result_t() noexcept {
        check_on_destroy_();
    }

    template <typename T, bool Lean>
    void result_t<T, Lean>::check_on_destroy_() const noexcept {
#ifndef NDEBUG
        if (!checked_ && std::holds_alternative<error_storage_t>(value_or_error_)) {
            detail::report_unchecked_result(__FILE__, __LINE__);
        }
#else
        (void)0;
#endif
    }

    template <typename T, bool Lean>
    bool result_t<T, Lean>::is_error() const noexcept {
        checked_ = true;
        return std::holds_alternative<error_storage_t>(value_or_error_);
    }

    template <typename T, bool Lean>
    bool result_t<T, Lean>::is_success() const noexcept {
        checked_ = true;
        return std::holds_alternative<value_type_t>(value_or_error_);
    }

    template <typename T, bool Lean>
    auto result_t<T, Lean>::error() const noexcept
        -> std::conditional_t<Lean, error_context_t, const error_context_t&> {
        checked_ = true;
        assert(std::holds_alternative<error_storage_t>(value_or_error_) && "result_t::error() called on a success result");
        if constexpr (Lean) {
            auto* code_ptr = std::get_if<error_code_t>(&value_or_error_);
            if (code_ptr) {
                return error_context_t::make_minimal(*code_ptr);
            }
            return error_context_t{};
        } else {
            auto* ptr = std::get_if<error_context_t>(&value_or_error_);
            if (ptr) {
                return *ptr;
            }
            static thread_local const error_context_t sentinel{};
            return sentinel;
        }
    }

    template <typename T, bool Lean>
    error_code_t result_t<T, Lean>::error_code() const noexcept {
        checked_ = true;
        if constexpr (Lean) {
            auto* code_ptr = std::get_if<error_code_t>(&value_or_error_);
            if (code_ptr) {
                return *code_ptr;
            }
        } else {
            auto* ctx_ptr = std::get_if<error_context_t>(&value_or_error_);
            if (ctx_ptr) {
                return ctx_ptr->get_code();
            }
        }
        return error_code_t::make_success();
    }

    template <typename T, bool Lean>
    template <bool L, typename>
    error_context_t& result_t<T, Lean>::error() noexcept {
        checked_ = true;
        assert(std::holds_alternative<error_storage_t>(value_or_error_) && "result_t::error() called on a success result");
        auto* ptr = std::get_if<error_context_t>(&value_or_error_);
        if (ptr) {
            return *ptr;
        }
        static thread_local error_context_t sentinel{};
        return sentinel;
    }

    template <typename T, bool Lean>
    const T& result_t<T, Lean>::value() const noexcept {
        static_assert(std::is_default_constructible_v<value_type_t>,
                      "result_t::value() requires T to be default-constructible. "
                      "Use value_pointer() for non-default-constructible types.");
        checked_ = true;
        assert(std::holds_alternative<value_type_t>(value_or_error_) && "result_t::value() called on an error result");
        auto* ptr = std::get_if<value_type_t>(&value_or_error_);
        if (ptr) {
            return *ptr;
        }
        static thread_local const value_type_t sentinel{};
        return sentinel;
    }

    template <typename T, bool Lean>
    T& result_t<T, Lean>::value() noexcept {
        static_assert(std::is_default_constructible_v<value_type_t>,
                      "result_t::value() requires T to be default-constructible. "
                      "Use value_pointer() for non-default-constructible types.");
        checked_ = true;
        assert(std::holds_alternative<value_type_t>(value_or_error_) && "result_t::value() called on an error result");
        auto* ptr = std::get_if<value_type_t>(&value_or_error_);
        if (ptr) {
            return *ptr;
        }
        static thread_local value_type_t sentinel{};
        return sentinel;
    }

    template <typename T, bool Lean>
    const T* result_t<T, Lean>::value_pointer() const noexcept {
        checked_ = true;
        return std::get_if<value_type_t>(&value_or_error_);
    }

    template <typename T, bool Lean>
    T* result_t<T, Lean>::value_pointer() noexcept {
        checked_ = true;
        return std::get_if<value_type_t>(&value_or_error_);
    }

    template <typename T, bool Lean>
    const T& result_t<T, Lean>::value_or(const value_type_t& default_value) const noexcept {
        checked_ = true;
        auto* ptr = std::get_if<value_type_t>(&value_or_error_);
        return ptr ? *ptr : default_value;
    }

    template <typename T, bool Lean>
    const T& result_t<T, Lean>::operator*() const noexcept {
        return value();
    }

    template <typename T, bool Lean>
    T& result_t<T, Lean>::operator*() noexcept {
        return value();
    }

    template <typename T, bool Lean>
    const T* result_t<T, Lean>::operator->() const noexcept {
        return value_pointer();
    }

    template <typename T, bool Lean>
    T* result_t<T, Lean>::operator->() noexcept {
        return value_pointer();
    }

    template <typename T, bool Lean>
    result_t<T, Lean>::operator bool() const noexcept {
        checked_ = true;
        return std::holds_alternative<value_type_t>(value_or_error_);
    }

    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::map(Function&& function) const& noexcept
        -> result_t<decltype(std::invoke(std::forward<Function>(function), std::declval<const value_type_t&>())), Lean> {
        using new_type = decltype(std::invoke(std::forward<Function>(function), std::declval<const value_type_t&>()));
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<new_type, Lean>(error_code());
            } else {
                return result_t<new_type, Lean>(error());
            }
        }
        try {
            return result_t<new_type, Lean>(std::invoke(std::forward<Function>(function), value()));
        } catch (...) {
            std::fprintf(stderr, "[result_t] map: std::invoke threw exception\n");
            return result_t<new_type, Lean>(detail::make_invoke_exception_context("map: function threw exception"));
        }
    }

    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::map(Function&& function) && noexcept
        -> result_t<decltype(std::invoke(std::forward<Function>(function), std::move(value()))), Lean> {
        using new_type = decltype(std::invoke(std::forward<Function>(function), std::move(value())));
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<new_type, Lean>(error_code());
            } else {
                return result_t<new_type, Lean>(std::move(error()));
            }
        }
        try {
            return result_t<new_type, Lean>(std::invoke(std::forward<Function>(function), std::move(value())));
        } catch (...) {
            std::fprintf(stderr, "[result_t] map(&&): std::invoke threw exception\n");
            return result_t<new_type, Lean>(detail::make_invoke_exception_context("map(&&): function threw exception"));
        }
    }

    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::map_error(Function&& function) const& noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(error_code());
            } else {
                try {
                    return result_t<T, Lean>(std::invoke(std::forward<Function>(function), error()));
                } catch (...) {
                    std::fprintf(stderr, "[result_t] map_error: std::invoke threw exception\n");
                    return result_t<T, Lean>(error());
                }
            }
        }
        return result_t<T, Lean>(value());
    }

    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::map_error(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(error_code());
            } else {
                auto error_copy = error();
                try {
                    return result_t<T, Lean>(std::invoke(std::forward<Function>(function), std::move(error())));
                } catch (...) {
                    std::fprintf(stderr, "[result_t] map_error(&&): std::invoke threw exception\n");
                    return result_t<T, Lean>(error_copy);
                }
            }
        }
        return std::move(*this);
    }

    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::and_then(Function&& function) && noexcept
        -> decltype(std::invoke(std::forward<Function>(function), std::move(value()))) {
        using return_type = decltype(std::invoke(std::forward<Function>(function), std::move(value())));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(error_code());
            } else {
                return return_type(std::move(error()));
            }
        }
        try {
            return std::invoke(std::forward<Function>(function), std::move(value()));
        } catch (...) {
            std::fprintf(stderr, "[result_t] and_then(&&): std::invoke threw exception\n");
            return return_type(detail::make_invoke_exception_context("and_then(&&): function threw exception"));
        }
    }

    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::and_then(Function&& function) & noexcept
        -> decltype(std::invoke(std::forward<Function>(function), value())) {
        using return_type = decltype(std::invoke(std::forward<Function>(function), value()));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(error_code());
            } else {
                return return_type(error());
            }
        }
        try {
            return std::invoke(std::forward<Function>(function), value());
        } catch (...) {
            std::fprintf(stderr, "[result_t] and_then(&): std::invoke threw exception\n");
            return return_type(detail::make_invoke_exception_context("and_then(&): function threw exception"));
        }
    }

    template <typename T, bool Lean>
    template <typename Function>
    auto result_t<T, Lean>::and_then(Function&& function) const& noexcept
        -> decltype(std::invoke(std::forward<Function>(function), value())) {
        using return_type = decltype(std::invoke(std::forward<Function>(function), value()));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(error_code());
            } else {
                return return_type(error());
            }
        }
        try {
            return std::invoke(std::forward<Function>(function), value());
        } catch (...) {
            std::fprintf(stderr, "[result_t] and_then(const&): std::invoke threw exception\n");
            return return_type(detail::make_invoke_exception_context("and_then(const&): function threw exception"));
        }
    }

    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::or_else(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(error_code());
            } else {
                auto error_copy = error();
                try {
                    return std::invoke(std::forward<Function>(function), std::move(error()));
                } catch (...) {
                    std::fprintf(stderr, "[result_t] or_else(&&): std::invoke threw exception\n");
                    return result_t<T, Lean>(error_copy);
                }
            }
        }
        return std::move(*this);
    }

    template <typename T, bool Lean>
    template <typename Function>
    result_t<T, Lean> result_t<T, Lean>::or_else(Function&& function) & noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<T, Lean>(error_code());
            } else {
                try {
                    return std::invoke(std::forward<Function>(function), error());
                } catch (...) {
                    std::fprintf(stderr, "[result_t] or_else(&): std::invoke threw exception\n");
                    return *this;
                }
            }
        }
        return *this;
    }

    template <typename T, bool Lean>
    template <typename SuccessFn, typename ErrorFn>
    auto result_t<T, Lean>::match(SuccessFn&& success_fn, ErrorFn&& error_fn) const
        noexcept(std::is_nothrow_invocable_v<SuccessFn&, const value_type_t&>
                 && std::is_nothrow_invocable_v<ErrorFn&, const error_context_t&>)
        -> decltype(success_fn(std::declval<const value_type_t&>())) {
        checked_ = true;
        if (std::holds_alternative<value_type_t>(value_or_error_)) {
            auto* ptr = std::get_if<value_type_t>(&value_or_error_);
            if (ptr) {
                return success_fn(*ptr);
            }
        } else {
            if constexpr (Lean) {
                auto* code_ptr = std::get_if<error_code_t>(&value_or_error_);
                if (code_ptr) {
                    return error_fn(error_context_t::make_minimal(*code_ptr));
                }
            } else {
                auto* ctx_ptr = std::get_if<error_context_t>(&value_or_error_);
                if (ctx_ptr) {
                    return error_fn(*ctx_ptr);
                }
            }
        }
        return {};
    }

    template <typename T, bool Lean>
    template <typename K, typename V, bool L, typename>
    result_t<T, Lean>& result_t<T, Lean>::context(K&& key, V&& value) & noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                error().with(std::forward<K>(key), std::forward<V>(value));
            }
        }
        return *this;
    }

    template <typename T, bool Lean>
    template <typename K, typename V, bool L, typename>
    result_t<T, Lean> result_t<T, Lean>::context(K&& key, V&& value) && noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                error().with(std::forward<K>(key), std::forward<V>(value));
            }
        }
        return std::move(*this);
    }

    /**
     * @brief result_t<void, Lean> 特化实现
     */

    template <bool Lean>
    inline result_t<void, Lean>::result_t(error_code_t code) noexcept
        : storage_(code) {}

    template <bool Lean>
    inline result_t<void, Lean>::result_t() noexcept : storage_{std::monostate{}} {}

    template <bool Lean>
    inline result_t<void, Lean>::result_t(const error_context_t& error_context) noexcept
        : storage_([&] {
            if constexpr (Lean) {
                return error_storage_t{error_context.get_code()};
            } else {
                return error_storage_t{error_context};
            }
        }()) {}

    template <bool Lean>
    inline result_t<void, Lean>::result_t(error_context_t&& error_context) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>)
        : storage_([&] {
            if constexpr (Lean) {
                return error_storage_t{error_context.get_code()};
            } else {
                return error_storage_t{std::move(error_context)};
            }
        }()) {}

    template <bool Lean>
    inline result_t<void, Lean>::result_t(const result_t& other) noexcept
        : storage_(other.storage_), checked_(false) {}

    template <bool Lean>
    inline result_t<void, Lean>::result_t(result_t&& other) noexcept
        : storage_(std::move(other.storage_)), checked_(other.checked_) {
        other.checked_ = true;
    }

    template <bool Lean>
    inline result_t<void, Lean>& result_t<void, Lean>::operator=(result_t&& other) noexcept {
        if (this != &other) {
            check_on_destroy_();
            storage_ = std::move(other.storage_);
            checked_ = other.checked_;
            other.checked_ = true;
        }
        return *this;
    }

    template <bool Lean>
    inline result_t<void, Lean>::~result_t() noexcept {
        check_on_destroy_();
    }

    template <bool Lean>
    inline void result_t<void, Lean>::check_on_destroy_() const noexcept {
#ifndef NDEBUG
        if (!checked_ && std::holds_alternative<error_storage_t>(storage_)) {
            detail::report_unchecked_result(__FILE__, __LINE__);
        }
#else
        (void)0;
#endif
    }

    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_error(error_code_t code, const std::string& message,
                                                                  utils::source_location_t location) noexcept {
        if constexpr (Lean) {
            (void)message;
            (void)location;
            return result_t<void, Lean>(code);
        } else {
            return result_t<void, Lean>(error_context_t{located_code_t{code, location}, message});
        }
    }

    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_error(error_code_t code, std::string&& message,
                                                                  utils::source_location_t location) noexcept {
        if constexpr (Lean) {
            (void)message;
            (void)location;
            return result_t<void, Lean>(code);
        } else {
            return result_t<void, Lean>(error_context_t{located_code_t{code, location}, std::move(message)});
        }
    }

    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_error(const error_context_t& context) noexcept {
        if constexpr (Lean) {
            return result_t<void, Lean>(context.get_code());
        } else {
            return result_t<void, Lean>(context);
        }
    }

    template <bool Lean>
    inline result_t<void, Lean> result_t<void, Lean>::make_success() noexcept {
        return result_t();
    }

    template <bool Lean>
    inline result_t<void, Lean>::operator bool() const noexcept {
        checked_ = true;
        return std::holds_alternative<std::monostate>(storage_);
    }

    template <bool Lean>
    inline bool result_t<void, Lean>::is_error() const noexcept {
        checked_ = true;
        return !std::holds_alternative<std::monostate>(storage_);
    }

    template <bool Lean>
    inline bool result_t<void, Lean>::is_success() const noexcept {
        checked_ = true;
        return std::holds_alternative<std::monostate>(storage_);
    }

    template <bool Lean>
    inline auto result_t<void, Lean>::error() const noexcept
        -> std::conditional_t<Lean, error_context_t, const error_context_t&> {
        checked_ = true;
        assert(std::holds_alternative<error_storage_t>(storage_) && "result_t<void>::error() called on a success result");
        if constexpr (Lean) {
            auto* code_ptr = std::get_if<error_code_t>(&storage_);
            if (code_ptr) {
                return error_context_t::make_minimal(*code_ptr);
            }
            return error_context_t{};
        } else {
            auto* ptr = std::get_if<error_context_t>(&storage_);
            if (ptr) {
                return *ptr;
            }
            static thread_local const error_context_t sentinel{};
            return sentinel;
        }
    }

    template <bool Lean>
    inline error_code_t result_t<void, Lean>::error_code() const noexcept {
        checked_ = true;
        if constexpr (Lean) {
            auto* code_ptr = std::get_if<error_code_t>(&storage_);
            if (code_ptr) {
                return *code_ptr;
            }
        } else {
            auto* ctx_ptr = std::get_if<error_context_t>(&storage_);
            if (ctx_ptr) {
                return ctx_ptr->get_code();
            }
        }
        return error_code_t::make_success();
    }

    template <bool Lean>
    template <bool L, typename>
    inline error_context_t& result_t<void, Lean>::error() noexcept {
        checked_ = true;
        assert(std::holds_alternative<error_storage_t>(storage_) && "result_t<void>::error() called on a success result");
        auto* ptr = std::get_if<error_context_t>(&storage_);
        if (ptr) {
            return *ptr;
        }
        static thread_local error_context_t sentinel{};
        return sentinel;
    }

    template <bool Lean>
    template <typename Function>
    auto result_t<void, Lean>::and_then(Function&& function) && noexcept
        -> decltype(std::invoke(std::forward<Function>(function))) {
        using return_type = decltype(std::invoke(std::forward<Function>(function)));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(error_code());
            } else {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    return return_type(std::move(*ptr));
                }
            }
        }
        try {
            return std::invoke(std::forward<Function>(function));
        } catch (...) {
            std::fprintf(stderr, "[result_t<void>] and_then(&&): std::invoke threw exception\n");
            return return_type(detail::make_invoke_exception_context("and_then(&&): function threw exception"));
        }
    }

    template <bool Lean>
    template <typename Function>
    auto result_t<void, Lean>::and_then(Function&& function) & noexcept
        -> decltype(std::invoke(std::forward<Function>(function))) {
        using return_type = decltype(std::invoke(std::forward<Function>(function)));
        if (is_error()) {
            if constexpr (Lean) {
                return return_type(error_code());
            } else {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    return return_type(*ptr);
                }
            }
        }
        try {
            return std::invoke(std::forward<Function>(function));
        } catch (...) {
            std::fprintf(stderr, "[result_t<void>] and_then(&): std::invoke threw exception\n");
            return return_type(detail::make_invoke_exception_context("and_then(&): function threw exception"));
        }
    }

    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::map_error(Function&& function) const& noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(error_code());
            } else {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    try {
                        return result_t<void, Lean>(std::invoke(std::forward<Function>(function), *ptr));
                    } catch (...) {
                        std::fprintf(stderr, "[result_t<void>] map_error: std::invoke threw exception\n");
                        return result_t<void, Lean>(*ptr);
                    }
                }
            }
        }
        return *this;
    }

    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::map_error(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(error_code());
            } else {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    try {
                        return result_t<void, Lean>(std::invoke(std::forward<Function>(function), std::move(*ptr)));
                    } catch (...) {
                        std::fprintf(stderr, "[result_t<void>] map_error(&&): std::invoke threw exception\n");
                        return result_t<void, Lean>(detail::make_invoke_exception_context("map_error(&&): function threw exception"));
                    }
                }
            }
        }
        return std::move(*this);
    }

    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::or_else(Function&& function) && noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(error_code());
            } else {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    try {
                        return std::invoke(std::forward<Function>(function), std::move(*ptr));
                    } catch (...) {
                        std::fprintf(stderr, "[result_t<void>] or_else(&&): std::invoke threw exception\n");
                        return std::move(*this);
                    }
                }
            }
        }
        return std::move(*this);
    }

    template <bool Lean>
    template <typename Function>
    result_t<void, Lean> result_t<void, Lean>::or_else(Function&& function) & noexcept {
        if (is_error()) {
            if constexpr (Lean) {
                return result_t<void, Lean>(error_code());
            } else {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    try {
                        return std::invoke(std::forward<Function>(function), *ptr);
                    } catch (...) {
                        std::fprintf(stderr, "[result_t<void>] or_else(&): std::invoke threw exception\n");
                        return *this;
                    }
                }
            }
        }
        return *this;
    }

    template <bool Lean>
    template <typename K, typename V, bool L, typename>
    result_t<void, Lean>& result_t<void, Lean>::context(K&& key, V&& value) & noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    ptr->with(std::forward<K>(key), std::forward<V>(value));
                }
            }
        }
        return *this;
    }

    template <bool Lean>
    template <typename K, typename V, bool L, typename>
    result_t<void, Lean> result_t<void, Lean>::context(K&& key, V&& value) && noexcept {
        if constexpr (!Lean) {
            if (is_error()) {
                auto* ptr = std::get_if<error_context_t>(&storage_);
                if (ptr) {
                    ptr->with(std::forward<K>(key), std::forward<V>(value));
                }
            }
        }
        return std::move(*this);
    }

}  // namespace error_system::core
