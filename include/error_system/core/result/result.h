#pragma once
#include <cassert>
#include <functional>
#include <new>
#include <string>
#include <string_view>
#include <type_traits>

#include "error_system/core/error_context.h"
#include "error_system/utils/log.h"
#include "error_system/utils/string_format.h"

/**
 * @file result.h
 * @brief 结果类型 result_t
 * @details 强制错误检查：Debug 构建下析构时若处于错误状态且未被检查（is_error/is_success/
 *          value/error/operator bool/value_pointer/value_or/match 任一未调用）触发 assert；
 *          Release 构建零开销。
 *
 *          存储方案：union + result_state_t 手写判别式替代 std::variant，以支持 Move-Only
 *          的 error_context_t（C++17 std::variant 要求元素可拷贝）。Lean=false 时 result_t
 *          整体为 Move-Only；Lean=true 时若 T 可拷贝则 result_t 可拷贝。
 *
 *          Lean 模式：error_storage_t 为 error_code_t（8B），零堆开销，体积最小。
 *          通知路径走 on_code(code)，不构造 error_context_t。
 * @author yiice
 * @version 4.4.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief result_t 内部状态判别式
     * @details 配合 union 使用，标记当前活跃成员。手写判别式避免 std::variant
     *          对元素可拷贝的硬性要求，从而支持 Move-Only 的 error_context_t。
     */
    enum class result_state_t : uint8_t {
        empty,
        value,
        error,
    };

    namespace detail {
        constexpr uint16_t FATAL_ERROR_NUMBER = 0xFFFE;

        inline void report_unchecked_result(const char* file, int line) noexcept {
            ::error_system::utils::log(::error_system::utils::log_level_t::error, file, line,
                                       "[result_t] unchecked error result destroyed at {}:{}\n"
                                       "  call is_error()/is_success()/value()/error()/operator bool/\n"
                                       "  value_pointer()/value_or()/match() before destruction.",
                                       file, line);
            assert(false && "unchecked error result destroyed");
        }

        /**
         * @brief 构造调用异常上下文
         * @param message 异常描述信息
         * @return 构造好的错误上下文，分配失败时返回空哨兵克隆
         */
        inline error_context_t make_invoke_exception_context(const char* message) noexcept {
            try {
                return error_context_t{located_code_t{error_code_t(error_level_t::fatal,
                                                                   domain::system_domain_t::none,
                                                                   subsystem_id_t{0},
                                                                   module_id_t{0},
                                                                   error_number_t{FATAL_ERROR_NUMBER})},
                                       message};
            } catch (const std::bad_alloc&) {
                LOG_ERROR("[result_t] make_invoke_exception_context: std::bad_alloc");
                static thread_local error_context_t fallback{};
                return fallback.clone();
            }
        }
    }  // namespace detail

    /**
     * @brief 结果类型
     * @details 封装成功值或错误上下文。Lean=true 时错误路径仅携带 error_code_t（无 message/
     *          payload/cause/stack），适用于热路径性能敏感场景。Lean 模式下 error() 返回
     *          值类型 error_context_t（临时构造仅含 code），context() 方法不可用。
     * @tparam T 结果类型
     * @tparam Lean 是否为精简模式
     */
    template <typename T, bool Lean = false>
    class result_t {
        public:
        using value_type_t = T;

        private:
        using error_storage_t = std::conditional_t<Lean, error_code_t, error_context_t>;

        template <typename OtherT, bool OtherLean>
        friend class result_t;

        result_state_t state_{result_state_t::value};
        union storage_t {
            value_type_t value;
            error_storage_t error;
            storage_t() noexcept {}
            ~storage_t() noexcept {}
        } storage_;
        mutable bool checked_{false};

        /**
         * @brief 析构当前活跃的 union 成员（empty 状态无操作）
         */
        void destroy_active_() noexcept;

        /**
         * @brief 析构前检查未消费错误
         * @details 仅 Debug 生效，Release 整段逻辑被消除
         */
        void check_on_destroy_() const noexcept;

        /**
         * @brief Lean 模式专用私有构造函数
         * @details 仅 Lean 模式启用，直接接受 error_code_t，零堆开销。
         * @param code 错误码
         */
        template <bool IsLean = Lean, typename = std::enable_if_t<IsLean>>
        explicit result_t(error_code_t code) noexcept;

        public:
        /**
         * @brief 拷贝构造（已删除，Move-Only 语义）
         * @param other 另一个结果对象
         */
        result_t(const result_t&) noexcept = delete;

        /**
         * @brief 拷贝赋值（已删除，Move-Only 语义）
         * @param other 另一个结果对象
         * @return 自身引用
         */
        result_t& operator=(const result_t&) noexcept = delete;

        /**
         * @brief 从成功值构造（左值版本）
         * @param value 成功值
         */
        explicit result_t(const value_type_t& value) noexcept(std::is_nothrow_copy_constructible_v<value_type_t>);

        /**
         * @brief 从成功值构造（右值版本）
         * @param value 成功值
         */
        explicit result_t(value_type_t&& value) noexcept(std::is_nothrow_move_constructible_v<value_type_t>);

        /**
         * @brief 从错误上下文构造
         * @param context 错误上下文
         */
        explicit result_t(error_context_t context) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 移动构造
         * @param other 另一个结果对象
         */
        result_t(result_t&& other) noexcept(std::is_nothrow_move_constructible_v<value_type_t> &&
                                            std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 移动赋值
         * @param other 另一个结果对象
         * @return 自身引用
         */
        result_t& operator=(result_t&& other) noexcept(std::is_nothrow_move_assignable_v<value_type_t> &&
                                                       std::is_nothrow_move_assignable_v<error_storage_t>);

        /**
         * @brief 析构函数
         * @details 检查未消费错误后析构活跃 union 成员
         */
        ~result_t() noexcept;

        /**
         * @brief 错误构造工厂（推荐替代直接构造错误结果，避免重载混淆）
         * @param code 错误码
         * @param message 错误消息
         * @param location 源位置
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t make_error(
            error_code_t code,
            const std::string& message = "",
            utils::source_location_t location =
                utils::source_location_t::current()) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 错误构造工厂（移动消息版本）
         * @param code 错误码
         * @param message 错误消息（右值，将被移动）
         * @param location 源位置
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t make_error(
            error_code_t code,
            std::string&& message,
            utils::source_location_t location =
                utils::source_location_t::current()) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 错误构造工厂（从 const error_context_t，完整模式下内部克隆）
         * @param context 错误上下文 const 引用
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t
        make_error(const error_context_t& context) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 错误构造工厂（从 error_context_t 右值，无克隆开销）
         * @param context 错误上下文右值
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t
        make_error(error_context_t&& context) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 获取成功值
         * @details 错误状态返回线程局部哨兵值（要求 T 可默认构造，否则用 value_pointer()）
         * @return 成功值 const 引用
         */
        [[nodiscard]] const value_type_t& value() const noexcept;

        /**
         * @brief 获取成功值（可变版本）
         * @details 错误状态返回线程局部哨兵值（要求 T 可默认构造，否则用 value_pointer()）
         * @return 成功值引用
         */
        [[nodiscard]] value_type_t& value() noexcept;

        /**
         * @brief 获取错误上下文
         * @details 完整模式返回 const 引用（成功状态返回线程局部哨兵值）；
         *          Lean 模式返回值类型 error_context_t（临时构造仅含 code）
         * @return 错误上下文
         */
        [[nodiscard]] auto error() const noexcept -> std::conditional_t<Lean, error_context_t, const error_context_t&>;

        /**
         * @brief 获取错误上下文可变引用
         * @details 仅完整模式可用，Lean 模式 SFINAE 禁用
         * @return 错误上下文可变引用
         */
        template <bool IsLean = Lean, typename = std::enable_if_t<!IsLean>>
        [[nodiscard]] error_context_t& error() noexcept;

        /**
         * @brief 获取错误码
         * @details Lean 模式直接返回存储值，完整模式从 error_context_t 提取
         * @return 错误码
         */
        [[nodiscard]] error_code_t error_code() const noexcept;

        /**
         * @brief 对成功值进行映射转换（const 左值版本）
         * @tparam Function 可调用对象类型，签名为 U(const value_type_t&)
         * @param function 处理成功值的可调用对象
         * @return 转换后的新 result_t
         */
        template <typename Function>
        [[nodiscard]] auto map(Function&& function) const& noexcept
            -> result_t<decltype(std::invoke(std::forward<Function>(function), std::declval<const value_type_t&>())),
                        Lean>;

        /**
         * @brief 对成功值进行映射转换（右值版本）
         * @tparam Function 可调用对象类型，签名为 U(value_type_t&&)
         * @param function 处理成功值的可调用对象
         * @return 转换后的新 result_t
         */
        template <typename Function>
        [[nodiscard]] auto map(Function&& function) && noexcept
            -> result_t<decltype(std::invoke(std::forward<Function>(function), std::move(value()))), Lean>;

        /**
         * @brief 对错误上下文进行映射转换（const 左值版本）
         * @tparam Function 可调用对象类型，签名为 error_context_t(const error_context_t&)
         * @param function 处理错误上下文的可调用对象
         * @return 转换后的新 result_t
         */
        template <typename Function>
        [[nodiscard]] result_t<value_type_t, Lean> map_error(Function&& function) const& noexcept;

        /**
         * @brief 对错误上下文进行映射转换（右值版本）
         * @tparam Function 可调用对象类型，签名为 error_context_t(error_context_t&&)
         * @param function 处理错误上下文的可调用对象
         * @return 转换后的新 result_t
         */
        template <typename Function>
        [[nodiscard]] result_t<value_type_t, Lean> map_error(Function&& function) && noexcept;

        /**
         * @brief 链式操作：成功时调用 function 处理值（右值版本，移动语义）
         * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
         * @param function 处理成功值的可调用对象
         * @return function 返回的结果
         */
        template <typename Function>
        [[nodiscard]] auto and_then(Function&& function) && noexcept
            -> decltype(std::invoke(std::forward<Function>(function), std::move(value())));

        /**
         * @brief 链式操作：成功时调用 function 处理值（左值版本，错误时克隆保持 *this 不变）
         * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
         * @param function 处理成功值的可调用对象
         * @return function 返回的结果
         */
        template <typename Function>
        [[nodiscard]] auto and_then(Function&& function) & noexcept
            -> decltype(std::invoke(std::forward<Function>(function), value()));

        /**
         * @brief 链式操作：成功时调用 function 处理值（const 左值版本，错误时克隆）
         * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
         * @param function 处理成功值的可调用对象
         * @return function 返回的结果
         */
        template <typename Function>
        [[nodiscard]] auto and_then(Function&& function) const& noexcept
            -> decltype(std::invoke(std::forward<Function>(function), value()));

        /**
         * @brief 错误时链式操作：错误时调用 function 处理错误（右值版本，移动语义）
         * @tparam Function 可调用对象类型，签名为 result_t(error_context_t&&)
         * @param function 处理错误的可调用对象
         * @return 成功时移动自身，错误时返回 function 处理后的结果
         */
        template <typename Function>
        [[nodiscard]] result_t<value_type_t, Lean> or_else(Function&& function) && noexcept;

        /**
         * @brief 错误时链式操作：错误时调用 function 处理错误（左值版本，成功时克隆）
         * @tparam Function 可调用对象类型，签名为 result_t(const error_context_t&)
         * @param function 处理错误的可调用对象
         * @return 成功时克隆自身，错误时返回 function 处理后的结果
         */
        template <typename Function>
        [[nodiscard]] result_t<value_type_t, Lean> or_else(Function&& function) & noexcept;

        /**
         * @brief 模式匹配：成功调用 success_function，错误调用 error_function
         * @details 两者须返回相同类型
         * @tparam SuccessFunction 成功路径可调用对象类型
         * @tparam ErrorFunction 错误路径可调用对象类型
         * @param success_function 成功时调用的可调用对象
         * @param error_function 错误时调用的可调用对象
         * @return 两个 function 返回类型的共同类型
         */
        template <typename SuccessFunction, typename ErrorFunction>
        [[nodiscard]] auto match(SuccessFunction&& success_function, ErrorFunction&& error_function) const
            noexcept(std::is_nothrow_invocable_v<SuccessFunction&, const value_type_t&> &&
                     std::is_nothrow_invocable_v<ErrorFunction&, const error_context_t&>)
                -> decltype(success_function(std::declval<const value_type_t&>()));

        /**
         * @brief 传播时附加 payload 上下文（左值版本）
         * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
         * @tparam Key 键类型
         * @tparam V 值类型
         * @param key 字段名
         * @param value 字段值
         * @return 自身引用（支持链式调用）
         */
        template <typename Key, typename V, bool IsLean = Lean, typename = std::enable_if_t<!IsLean>>
        result_t& context(Key&& key, V&& value) & noexcept;

        /**
         * @brief 传播时附加 payload 上下文（右值版本，返回移动后的新对象）
         * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
         * @tparam Key 键类型
         * @tparam V 值类型
         * @param key 字段名
         * @param value 字段值
         * @return 移动后的新结果对象
         */
        template <typename Key, typename V, bool IsLean = Lean, typename = std::enable_if_t<!IsLean>>
        [[nodiscard]] result_t context(Key&& key, V&& value) && noexcept;

        /**
         * @brief 是否为错误状态
         * @return 错误状态返回 true，否则 false
         */
        [[nodiscard]] bool is_error() const noexcept {
            checked_ = true;
            return state_ == result_state_t::error;
        }

        /**
         * @brief 是否为成功状态
         * @return 成功状态返回 true，否则 false
         */
        [[nodiscard]] bool is_success() const noexcept {
            checked_ = true;
            return state_ == result_state_t::value;
        }

        /**
         * @brief 显式转换为 bool
         * @return 成功返回 true，错误返回 false
         */
        explicit operator bool() const noexcept {
            checked_ = true;
            return state_ == result_state_t::value;
        }

        /**
         * @brief 安全获取成功值指针（const 版本）
         * @details 错误时返回 nullptr
         * @return 成功值 const 指针，错误时返回 nullptr
         */
        [[nodiscard]] const value_type_t* value_pointer() const noexcept {
            checked_ = true;
            if (state_ == result_state_t::value) {
                return &storage_.value;
            }
            return nullptr;
        }

        /**
         * @brief 安全获取成功值指针（可变版本）
         * @details 错误时返回 nullptr
         * @return 成功值指针，错误时返回 nullptr
         */
        [[nodiscard]] value_type_t* value_pointer() noexcept {
            checked_ = true;
            if (state_ == result_state_t::value) {
                return &storage_.value;
            }
            return nullptr;
        }

        /**
         * @brief 获取成功值，失败时返回 default_value
         * @details 调用方保证 default_value 的生命周期
         * @param default_value 默认值
         * @return 成功值引用，失败时返回 default_value
         */
        [[nodiscard]] const value_type_t& value_or(const value_type_t& default_value) const noexcept {
            checked_ = true;
            if (state_ == result_state_t::value) {
                return storage_.value;
            }
            return default_value;
        }

        /**
         * @brief 解引用（const 版本）
         * @details 错误时返回线程局部哨兵值引用，与 std::expected::operator* 语义一致
         * @return 成功值 const 引用
         */
        [[nodiscard]] const value_type_t& operator*() const noexcept { return value(); }

        /**
         * @brief 解引用（可变版本）
         * @details 错误时返回线程局部哨兵值引用，与 std::expected::operator* 语义一致
         * @return 成功值引用
         */
        [[nodiscard]] value_type_t& operator*() noexcept { return value(); }

        /**
         * @brief 箭头访问（const 版本）
         * @details 错误时返回 nullptr，与 std::expected::operator-> 语义一致
         * @return 成功值 const 指针，错误时返回 nullptr
         */
        [[nodiscard]] const value_type_t* operator->() const noexcept { return value_pointer(); }

        /**
         * @brief 箭头访问（可变版本）
         * @details 错误时返回 nullptr，与 std::expected::operator-> 语义一致
         * @return 成功值指针，错误时返回 nullptr
         */
        [[nodiscard]] value_type_t* operator->() noexcept { return value_pointer(); }

        /**
         * @brief 转换为字符串描述
         * @details 成功返回 "[OK: <value>]"（若 T 可被 string_format_t 格式化），
         *          错误返回 "[ERR: <context>]"（Full 模式委托 error_context_t::to_string，
         *          Lean 模式输出 code @ file:line），empty 返回 "[empty]"。
         *          提供本方法后 result_t 可直接作为 string_format_t 的 "{}" 参数。
         * @return 状态描述字符串
         */
        [[nodiscard]] std::string to_string() const noexcept;

        /**
         * @brief 成功结果工厂
         * @param value 成功值
         * @return 构造好的成功结果
         */
        [[nodiscard]] static result_t
        make_success(value_type_t value) noexcept(std::is_nothrow_move_constructible_v<value_type_t>) {
            return result_t(std::move(value));
        }
    };

    /**
     * @brief result_t<void> 特化：不包含成功值
     * @details 成功路径 state_ 为 empty（零开销），失败路径持有 error_storage_t。
     *          强制错误检查与主模板一致。
     * @tparam Lean 是否为精简模式，与主模板语义一致
     */
    template <bool Lean>
    class result_t<void, Lean> {
        public:
        using value_type_t = void;

        private:
        using error_storage_t = std::conditional_t<Lean, error_code_t, error_context_t>;

        template <typename OtherT, bool OtherLean>
        friend class result_t;

        result_state_t state_{result_state_t::empty};
        union storage_t {
            error_storage_t error;
            storage_t() noexcept {}
            ~storage_t() noexcept {}
        } storage_;
        mutable bool checked_{false};

        /**
         * @brief 析构当前活跃的 union 成员（empty 状态无操作）
         */
        void destroy_active_() noexcept;

        /**
         * @brief 析构前检查未消费错误
         * @details 仅 Debug 生效，Release 整段逻辑被消除
         */
        void check_on_destroy_() const noexcept;

        /**
         * @brief Lean 模式专用私有构造函数
         * @details 仅 Lean 模式启用，直接接受 error_code_t，零堆开销。
         * @param code 错误码
         */
        template <bool IsLean = Lean, typename = std::enable_if_t<IsLean>>
        explicit result_t(error_code_t code) noexcept;

        public:
        /**
         * @brief 拷贝构造（已删除，Move-Only 语义）
         * @param other 另一个结果对象
         */
        result_t(const result_t&) noexcept = delete;

        /**
         * @brief 拷贝赋值（已删除，Move-Only 语义）
         * @param other 另一个结果对象
         * @return 自身引用
         */
        result_t& operator=(const result_t&) noexcept = delete;

        /**
         * @brief 默认构造（成功状态）
         * @return 无返回值（构造函数）
         */
        result_t() noexcept;

        /**
         * @brief 从错误上下文构造
         * @param context 错误上下文
         */
        explicit result_t(error_context_t context) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 移动构造
         * @param other 另一个结果对象
         */
        result_t(result_t&& other) noexcept(std::is_nothrow_move_constructible_v<error_storage_t>);

        /**
         * @brief 移动赋值
         * @param other 另一个结果对象
         * @return 自身引用
         */
        result_t& operator=(result_t&& other) noexcept(std::is_nothrow_move_assignable_v<error_storage_t>);

        /**
         * @brief 析构函数
         * @details 检查未消费错误后析构活跃 union 成员
         */
        ~result_t() noexcept;

        /**
         * @brief 错误构造工厂
         * @param code 错误码
         * @param message 错误消息
         * @param location 源位置
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t
        make_error(error_code_t code,
                   const std::string& message = "",
                   utils::source_location_t location = utils::source_location_t::current()) noexcept;

        /**
         * @brief 错误构造工厂（移动消息版本）
         * @param code 错误码
         * @param message 错误消息（右值，将被移动）
         * @param location 源位置
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t
        make_error(error_code_t code,
                   std::string&& message,
                   utils::source_location_t location = utils::source_location_t::current()) noexcept;

        /**
         * @brief 错误构造工厂（从 const error_context_t，完整模式下内部克隆）
         * @param context 错误上下文 const 引用
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t make_error(const error_context_t& context) noexcept;

        /**
         * @brief 错误构造工厂（从 error_context_t 右值，无克隆开销）
         * @param context 错误上下文右值
         * @return 构造好的错误结果
         */
        [[nodiscard]] static result_t make_error(error_context_t&& context) noexcept;

        /**
         * @brief 获取错误上下文
         * @details 完整模式返回 const 引用（成功状态返回线程局部哨兵值）；
         *          Lean 模式返回值类型 error_context_t（临时构造仅含 code）
         * @return 错误上下文
         */
        [[nodiscard]] auto error() const noexcept -> std::conditional_t<Lean, error_context_t, const error_context_t&>;

        /**
         * @brief 获取错误上下文可变引用
         * @details 仅完整模式可用，Lean 模式 SFINAE 禁用
         * @return 错误上下文可变引用
         */
        template <bool IsLean = Lean, typename = std::enable_if_t<!IsLean>>
        [[nodiscard]] error_context_t& error() noexcept;

        /**
         * @brief 获取错误码
         * @details Lean 模式直接返回存储值，完整模式从 error_context_t 提取
         * @return 错误码
         */
        [[nodiscard]] error_code_t error_code() const noexcept;

        /**
         * @brief 链式操作：成功时调用 function 处理（右值版本，移动语义）
         * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
         * @param function 处理成功路径的可调用对象
         * @return function 返回的结果
         */
        template <typename Function>
        [[nodiscard]] auto and_then(Function&& function) && noexcept
            -> decltype(std::invoke(std::forward<Function>(function)));

        /**
         * @brief 链式操作：成功时调用 function 处理（左值版本，错误时克隆）
         * @tparam Function 可调用对象类型，返回 result_t<U, Lean>
         * @param function 处理成功路径的可调用对象
         * @return function 返回的结果
         */
        template <typename Function>
        [[nodiscard]] auto and_then(Function&& function) & noexcept
            -> decltype(std::invoke(std::forward<Function>(function)));

        /**
         * @brief 对错误上下文进行映射转换（const 左值版本）
         * @tparam Function 可调用对象类型，签名为 error_context_t(const error_context_t&)
         * @param function 处理错误上下文的可调用对象
         * @return 转换后的新 result_t
         */
        template <typename Function>
        [[nodiscard]] result_t<void, Lean> map_error(Function&& function) const& noexcept;

        /**
         * @brief 对错误上下文进行映射转换（右值版本）
         * @tparam Function 可调用对象类型，签名为 error_context_t(error_context_t&&)
         * @param function 处理错误上下文的可调用对象
         * @return 转换后的新 result_t
         */
        template <typename Function>
        [[nodiscard]] result_t<void, Lean> map_error(Function&& function) && noexcept;

        /**
         * @brief 错误时链式操作：错误时调用 function 处理错误（右值版本，移动语义）
         * @tparam Function 可调用对象类型，签名为 result_t(error_context_t&&)
         * @param function 处理错误的可调用对象
         * @return 成功时移动自身，错误时返回 function 处理后的结果
         */
        template <typename Function>
        [[nodiscard]] result_t<void, Lean> or_else(Function&& function) && noexcept;

        /**
         * @brief 错误时链式操作：错误时调用 function 处理错误（左值版本，成功时克隆）
         * @tparam Function 可调用对象类型，签名为 result_t(const error_context_t&)
         * @param function 处理错误的可调用对象
         * @return 成功时克隆自身，错误时返回 function 处理后的结果
         */
        template <typename Function>
        [[nodiscard]] result_t<void, Lean> or_else(Function&& function) & noexcept;

        /**
         * @brief 传播时附加 payload 上下文（左值版本）
         * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
         * @tparam Key 键类型
         * @tparam V 值类型
         * @param key 字段名
         * @param value 字段值
         * @return 自身引用（支持链式调用）
         */
        template <typename Key, typename V, bool IsLean = Lean, typename = std::enable_if_t<!IsLean>>
        result_t& context(Key&& key, V&& value) & noexcept;

        /**
         * @brief 传播时附加 payload 上下文（右值版本，返回移动后的新对象）
         * @details 完美转发到 error_context_t::with()。仅完整模式可用，Lean 模式 SFINAE 禁用。
         * @tparam Key 键类型
         * @tparam V 值类型
         * @param key 字段名
         * @param value 字段值
         * @return 移动后的新结果对象
         */
        template <typename Key, typename V, bool IsLean = Lean, typename = std::enable_if_t<!IsLean>>
        [[nodiscard]] result_t context(Key&& key, V&& value) && noexcept;

        /**
         * @brief 显式转换为 bool
         * @return 成功返回 true，错误返回 false
         */
        explicit operator bool() const noexcept {
            checked_ = true;
            return state_ == result_state_t::empty;
        }

        /**
         * @brief 是否为错误状态
         * @return 错误状态返回 true，否则 false
         */
        [[nodiscard]] bool is_error() const noexcept {
            checked_ = true;
            return state_ == result_state_t::error;
        }

        /**
         * @brief 是否为成功状态
         * @return 成功状态返回 true，否则 false
         */
        [[nodiscard]] bool is_success() const noexcept {
            checked_ = true;
            return state_ == result_state_t::empty;
        }

        /**
         * @brief 转换为字符串描述
         * @details 成功返回 "[OK]"，错误返回 "[ERR: <context>]"（Full 模式委托
         *          error_context_t::to_string，Lean 模式输出 code @ file:line）。
         *          提供本方法后 result_t 可直接作为 string_format_t 的 "{}" 参数。
         * @return 状态描述字符串
         */
        [[nodiscard]] std::string to_string() const noexcept;

        /**
         * @brief 成功结果工厂
         * @return 构造好的成功结果
         */
        [[nodiscard]] static result_t make_success() noexcept { return result_t(); }
    };

}  // namespace error_system::core

/**
 * @brief 早返回宏：当 expr 为错误时，从当前函数返回同类型错误结果
 * @details 声明变量 var 并赋值为 expr；若 var 为错误，则构造与 var 相同 value 类型的错误结果并返回。
 *          外层函数返回类型必须与 expr 的 value 类型一致。Move-Only 适配：error() 返回 const 引用，
 *          make_error 内部按需克隆。
 * @param var 变量名（在调用点可见）
 * @param expr 返回 result_t 的表达式
 * @note 该宏展开为多语句，必须作为独立语句使用
 */
#ifndef ERROR_SYSTEM_TRY
#define ERROR_SYSTEM_TRY(var, expr)                                                                                    \
    auto var = (expr);                                                                                                 \
    if ((var).is_error()) {                                                                                            \
        return ::error_system::core::result_t<typename std::decay_t<decltype(var)>::value_type_t>::make_error(         \
            (var).error());                                                                                            \
    }
#endif

/**
 * @brief 早返回宏（丢弃值版本）：当 expr 为错误时，从当前函数返回同类型错误结果
 * @details 不保留成功值，仅检查错误并早返回。外层函数返回类型必须与 expr 的 value 类型一致。
 * @param expr 返回 result_t 的表达式
 */
#ifndef ERROR_SYSTEM_TRY_DISCARD
#define ERROR_SYSTEM_TRY_DISCARD(expr)                                                                                 \
    do {                                                                                                               \
        auto&& _error_system_tmp = (expr);                                                                             \
        if (_error_system_tmp.is_error()) {                                                                            \
            return ::error_system::core::result_t<typename std::decay_t<decltype(_error_system_tmp)>::value_type_t>::  \
                make_error(_error_system_tmp.error());                                                                 \
        }                                                                                                              \
    } while (0)
#endif

#include "details/result.inl"
