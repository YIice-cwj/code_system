# Error System

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-green.svg)](https://cmake.org)
[![Tests](https://img.shields.io/badge/Tests-706%20passing-brightgreen.svg)](https://github.com/google/googletest)

## 🚀 项目简介

高性能 C++17 错误码管理系统——将完整错误上下文（错误码、消息、负载、因果链、堆栈）压缩进一个 64 位整数与 24 字节的 Move-Only 上下文对象，成功路径近零开销，错误路径一次性付出 ~65 ns 换取结构化诊断信息。

## ⚡ 关键特性

- **constexpr 全链路**：`error_code_t` 构造、解析、哈希均为 constexpr，编译期常量进入只读数据段
- **64 位位移编码**：位移 + 掩码 100% 规避位域 UB，3 位 Reserved 携带 retryable/transient 语义
- **24 字节 Move-Only 上下文**：`error_context_t` 动静分离，`runtime_block_t` 按需分配，≤4 项 payload SSO 栈存储零堆分配
- **类 Rust `result_t<T, Lean>`**：`union` + 手写判别式替代 `std::variant`，支持 Move-Only 错误载荷；`map`/`and_then`/`or_else`/`match` 链式
- **Lean 模式**：`result_t<T, true>` 错误存储仅 8B `error_code_t`，通知走 `on_code(code)` 零上下文构造
- **插件系统**：RCU 无锁注册表、同步/异步/延迟三种通知模式、双通道（context + code）独立工作线程
- **因果链 `wrap()`**：`unique_ptr` 独占所有权，零引用计数，递归序列化
- **Per-Code 堆栈追踪**：全局阈值 + 按错误码粒度覆盖，延迟符号化 + `thread_local` 缓存
- **i18n 多语言**：两级哈希 + locale parent 链回退，15 种内置 locale
- **HTTP/gRPC 映射**：constexpr 纯函数，retryable/transient 优先映射 503/UNAVAILABLE
- **JSON → C++ 代码生成**：配置一键生成错误码头文件 + O(1) 字典 + 协作文档
- **零异常安全**：所有公共方法 `noexcept`，`std::bad_alloc` 内部捕获，Debug 构建强制错误检查（Release 零开销）

## 🛠️ 兼容性与依赖

| 项 | 要求 |
|------|------|
| C++ 标准 | C++17（不使用 C++20 特性） |
| GCC | ≥ 9 |
| Clang | ≥ 10 |
| MSVC | ≥ 19.20 |
| CMake | ≥ 3.15 |
| Python | ≥ 3.6（可选，错误码生成） |
| 第三方依赖 | GoogleTest 1.14.0、Google Benchmark 1.8.3（均 FetchContent，仅测试/基准） |

编译期特性开关（均默认 ON）：`ERROR_SYSTEM_ENABLE_STACKTRACE` / `VALIDATION` / `LOCATION`；调试选项：`LTO` / `PGO_GENERATE` / `PGO_USE` / `ASAN` / `UBSAN`。

## 📦 快速集成

### 源码构建

```bash
git clone https://github.com/YIice-cwj/error_system.git && cd error_system
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure  # 可选
```

### CMake FetchContent

```cmake
include(FetchContent)
FetchContent_Declare(error_system
    GIT_REPOSITORY https://github.com/YIice-cwj/error_system.git
    GIT_TAG v4.4.0)
FetchContent_MakeAvailable(error_system)
target_link_libraries(my_app PRIVATE error_system::error_system)
```

### 嵌入式最小化构建

```bash
cmake -S . -B build-min -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DERROR_SYSTEM_ENABLE_STACKTRACE=OFF -DERROR_SYSTEM_ENABLE_LOCATION=OFF \
    -DERROR_SYSTEM_BUILD_TESTS=OFF -DERROR_SYSTEM_BUILD_EXAMPLES=OFF
```

## 💡 核心示例

```cpp
#include "error_system.h"
using namespace error_system;

// 1. 编译期构建错误码（进入只读数据段）
constexpr auto db_err = error_code_t{
    core::error_level_t::fatal, domain::system_domain_t::database,
    core::subsystem_id_t{100}, core::module_id_t{200}, core::error_number_t{404}};

// 2. 错误上下文 + 结构化负载 + 因果链
core::error_context_t ctx{db_err, "数据库连接失败: {}", "timeout"};
ctx.with("host", "192.168.1.100").with("port", 3306);
auto chained = biz_err.wrap(std::move(db_ctx));   // biz_err ← db_ctx

core::error_context_serializer_t::to_string(ctx);  // 人类可读文本
core::error_context_serializer_t::to_json(ctx);    // JSON（含 cause 递归）

// 3. Result 错误传递（成功路径近零开销）
result_t<int> divide(int a, int b) {
    if (b == 0)
        return result_t<int>::make_error(db_err, "除数不能为零");
    return result_t<int>::make_success(a / b);
}

auto r = divide(10, 0);
if (!r) std::cerr << r.error().to_string();
r.value_or(0);  // 失败返回默认值

// 4. Lean 模式热路径（24B，on_code 通知不构造 context）
result_t<int, true> fast_op(int x) {
    if (x < 0) return result_t<int, true>::make_error(db_err);
    return result_t<int, true>::make_success(x * 2);
}

// 5. 插件注册
class logger_t : public plugin::i_error_plugin_t {
    std::string_view name() const noexcept override { return "logger"; }
    core::error_level_t min_level() const noexcept override { return core::error_level_t::error; }
    void on_error(const core::error_context_t& ctx) noexcept override {
        std::cerr << ctx.to_string() << "\n";
    }
    void on_code(core::error_code_t code) noexcept override {
        std::cerr << "[ERR: " << code.get_code() << "]\n";
    }
};
logger_t logger;
plugin::plugin_registry_t::instance().register_plugin_ref(logger);
```

## 📊 性能表现

基于 `tests/migration/perf/plain_error_code_benchmark.cc`，对比 plain int / `std::error_code` / error_system。10 次重复取中位数，macOS Apple Silicon 10 核，Release `-O3 -DNDEBUG` / Debug `-g`，全局关闭 validation/stacktrace/location。

| 维度 | plain int | std::error_code | error_system (Release) | error_system (Debug) |
|------|--------:|--------:|--------:|--------:|
| 成功构造 | 0.23 ns | 0.23 ns | 0.35 ns | 25.6 ns |
| 错误构造 | 0.23 ns | 0.23 ns | 65 ns | 1552 ns |
| 成功传播 | 0.23 ns | — | 0.47 ns | 65.5 ns |
| 错误传播 | 0.23 ns | — | 71 ns | 1656 ns |
| `to_string()` | 37 ns (snprintf) | — | 146 ns | 1209 ns |
| `to_json()` | — | — | 152 ns | 996 ns |

**sizeof 对比**：`int`=4B · `std::error_code`=16B · `error_context_t`=**24B** · `result_t<int>`=40B · `result_t<int, true>`=**24B**

成功路径 Release 下 `result_t` 比 plain int 仅多 0.12 ns，可忽略；错误路径 65 ns 换取消息/位置/负载/因果链/堆栈等 plain int 无法提供的能力。完整对比与运行方法见 [基准对比](docs/benchmark_comparison.md)。

## 文档

| 文档 | 内容 |
|------|------|
| [Core API](docs/api/core.md) | `error_code_t` `error_context_t` `result_t` `error_registry_t` |
| [Config API](docs/api/config.md) | 特性开关、堆栈/格式化器/i18n 配置 |
| [i18n API](docs/api/i18n.md) | 多语言消息目录与 locale 回退 |
| [Mapping API](docs/api/mapping.md) | HTTP / gRPC 状态码映射 |
| [Migration API](docs/api/migration.md) | 错误码废弃与迁移 |
| [Plugin API](docs/api/plugin.md) | 插件接口、注册表、路由、异步通知 |
| [Utils API](docs/api/utils.md) | 字符串、JSON、文件、异步队列、堆栈 |
| [Bridge API](docs/api/bridge.md) | C ABI 导出、std::error_code 桥接 |
| [Async API](docs/api/async.md) | async_result_t 链式异步错误处理 |
| [架构设计](docs/architecture.md) | 分层架构、关键设计决策、编译配置 |
| [决策树](docs/decision_tree.md) | 通知模式、查询路径、序列化等选型指南 |
| [错误码生成](docs/error_code_generation.md) | JSON 配置 → C++ 头文件 + 字典 + 文档 |
| [基准对比](docs/benchmark_comparison.md) | plain int / std::error_code / error_system 性能对比 |
| [系统配置指南](docs/configuration_guide.md) | 配置项总览、Lean/Full 选择、场景推荐 |

## 示例

| 示例 | 演示 |
|------|------|
| [demo01.cc](examples/demo01.cc) | 基础用法 |
| [demo02.cc](examples/demo02.cc) | Result 链式操作 |
| [demo03.cc](examples/demo03.cc) | 插件系统 |
| [demo04.cc](examples/demo04.cc) | 序列化与跨进程传递 |
| [demo05.cc](examples/demo05.cc) | 自动生成错误码 |
| [demo06.cc](examples/demo06.cc) | 高级用法综合示例 |

## License

MIT
