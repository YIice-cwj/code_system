# Async 层 API

基于 `std::future<result_t<T>>` 的 monadic 链式异步错误处理，为 C++17 提供 `then` / `recover` 链式 API。

## async_result_t\<T, bool Lean\>

封装 `std::future<result_t<T, Lean>>`，提供链式转换与错误恢复。`then` / `recover` 为 rvalue-qualified（消费语义），临时对象自动链式传递。

### 类型与构造

| 成员 | 说明 |
|------|------|
| `value_type_t` | `T` |
| `result_type_t` | `result_t<T, Lean>` |

| 构造 | 签名 | 说明 |
|------|------|------|
| 默认 | `async_result_t() noexcept` | 空状态 |
| future | `explicit async_result_t(std::future<result_type_t>) noexcept` | 从 future 构造 |
| result | `explicit async_result_t(result_type_t) noexcept` | 从 result 构造（同步包装） |
| 拷贝 | `= delete` | 不可拷贝 |
| 移动 | `= default` | 可移动 |

### 方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `get` | `[[nodiscard]] result_type_t get()` | 阻塞等待并获取结果（永不抛出，异常已转换为 fatal 错误） |
| `wait` | `void wait() const noexcept` | 阻塞等待就绪 |
| `is_ready` | `[[nodiscard]] bool is_ready() const noexcept` | 非阻塞检查是否就绪 |
| `valid` | `[[nodiscard]] bool valid() const noexcept` | 检查 future 是否有效 |
| `then` | `[[nodiscard]] auto then(Function&&) &&` | 链式转换：Function 接收 result，返回新 result；始终调用 |
| `recover` | `[[nodiscard]] async_result_t<T, Lean> recover(Function&&) &&` | 错误恢复：Function 接收 error_context_t，仅错误时调用。Lean 模式下传入 `make_minimal(code)`（无 file:line） |

### make_async

```cpp
template <typename Function>
[[nodiscard]] auto make_async(Function&& function)
    -> async_result_t<result_value_t<...>, result_lean_v<...>>;
```

从可调用对象创建异步结果。Lean 参数从返回类型自动推导，底层使用 `std::async(std::launch::async)`。

### 示例

```cpp
using namespace error_system;

auto result = async::make_async([] {
    return result_t<int>::make_success(42);
})
.then([](result_t<int> r) {
    return result_t<int>::make_success(r.value() * 2);
})
.recover([](error_context_t ctx) {
    return result_t<int>::make_success(0);  // 错误时恢复为 0
})
.get();  // → 84
```

### result_traits

类型萃取工具，从 `result_t<T, Lean>` 提取 `T` 与 `Lean`：

| 成员 | 说明 |
|------|------|
| `result_traits<R>::value_type` | 结果值类型 T |
| `result_traits<R>::lean` | 是否 Lean 模式 |
| `result_value_t<R>` | = `result_traits<R>::value_type` |
| `result_lean_v<R>` | = `result_traits<R>::lean` |
