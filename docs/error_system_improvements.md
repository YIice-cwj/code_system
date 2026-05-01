# Error System 架构优化建议与痛点分析

本文档旨在总结当前 `error_system` 基础设施中存在的架构痛点，并提供面向高并发、高性能及现代 C++ 范式的进阶改进方案。

## ⚠️ 痛点 1：`result_t` 缺乏现代 Monadic 操作（链式调用）

**现状分析**：
目前使用 `result_t` 必须写样板代码 `if (res.is_error()) return res.error();`。虽然使用了宏来简化，但在现代 C++（参考 C++23 `std::expected` 或 Rust `Result`）中，更流行的是**函数式链式调用**。

**💡 改进建议**：
在 `result_t` 中补充 `.and_then()` 和 `.or_else()` 方法。

```cpp
// 改进后的未来用法，彻底消除 if-else
auto res = read_model_weights("path")
           .and_then([](auto data) { return parse_weights(data); })
           .and_then([](auto tensor) { return init_engine(tensor); })
           .or_else([](auto err) { 
               return err.wrap(error_context_t(CODE, "流水线崩溃")); 
           });
```

## ⚠️ 痛点 2：单例注册表的线程安全隐患
**现状分析**：
translator_registry_t 和 plugin_registry_t 使用了 static 局部变量实现 Meyer's Singleton，保证了初始化的线程安全。但是，其内部的 set() 和 get() 方法没有加锁。如果在高并发运行期间（例如 Web 服务器处理请求时）触发了语言包热切换，极易引发 Data Race 和段错误。

**💡 改进建议**：
在 Registry 中引入 C++17 的 std::shared_mutex。get() 使用共享锁（读锁），set() 使用独占锁（写锁），确保热更新字典时绝对安全。

## ⚠️ 痛点 3：error_context_t 包含隐式堆分配性能陷阱
**现状分析**：
错误链 cause 使用了 std::shared_ptr<error_context_t>。每次调用 .wrap() 时，都会触发 std::make_shared，产生一次 Heap Allocation（堆分配）。在超高频核心热循环中（如高频交易或游戏渲染），堆分配的开销是致命的。

**💡 改进建议**：
无需彻底推翻重做，可通过文档规约明确界限：
告知使用者在核心热循环 (Hot Path) 中，仅传递 error_code_t （8字节纯值类型，完全跑在寄存器里，零开销）；只有在抛出到非热点业务层时，才包装成 error_context_t 进行错误链收集。

## ⚠️ 痛点 4：缺乏真正的“堆栈回溯 (Stacktrace)”
**现状分析**：
目前的错误链是靠开发者手动 .wrap() 包装。如果中间某一层忘了写 wrap() 直接抛出去了，案发现场就会断裂，难以定位真实的崩溃代码行。

**💡 改进建议**：
利用 C++23 的 <stacktrace>（或引入第三方轻量库 cpptrace / boost::stacktrace）。在 error_context_t 的构造函数中直接抓取当前的 C++ 调用栈，实现降维打击。

```cpp
#include <stacktrace> // C++23
struct error_context_t {
    error_code_t code{};
    std::string message{};
    std::stacktrace trace; // 实例化时自动捕获当前堆栈

    error_context_t(error_code_t c, std::string msg = "") noexcept 
        : code(c), message(std::move(msg)), trace(std::stacktrace::current()) {}
};
```
## ⚠️ 痛点 5：message 纯文本不利于机器解析 (Structured Logging)
**现状分析**：
在微服务架构中，错误信息往往发给 ELK (Elasticsearch) 进行聚合。如果 error_context_t 里只有拼装好的 std::string message（例如："未找到文件 user.txt"），机器很难按特定维度（如 FilePath、UserID）进行聚类和统计分析。

**💡 改进建议**：
为 error_context_t 增加一个轻量的键值对上下文负载 (Payload)，专门存放结构化数据：

```cpp
struct error_context_t {
    error_code_t code{};
    std::string message{};
    std::unordered_map<std::string, std::string> payload{}; // 结构化负载
    
    // 注入结构化数据，串联分布式请求
    error_context_t& with(std::string key, std::string val) {
        payload[std::move(key)] = std::move(val);
        return *this;
    }
};
```
// 用法示例：
// return err.with("user_id", "9527").with("trace_id", "abc-123");