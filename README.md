# Error System

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-green.svg)](https://cmake.org)
[![Tests](https://img.shields.io/badge/Tests-666%20passing-brightgreen.svg)](https://github.com/google/googletest)

高性能 C++17 错误码管理系统 — 将完整错误上下文封装在一个 64 位整数中，零开销构建与解析。

## 核心特性

- `constexpr` 全链路，编译期构建进入只读数据段
- 64 位位域编码，位移 + 掩码规避位域 UB
- 结构化负载 `with()` / `with_batch()`，SSO 栈上零堆分配
- Per-Code 堆栈追踪，全局阈值 + 按错误码粒度覆盖
- 插件系统：同步/异步通知、级别过滤、RCU 零拷贝
- 类 Rust `result_t<T>`，map / and_then / match 链式
- 因果链 `wrap()`，多层递归序列化
- JSON 配置一键生成 C++ 头文件 + 字典 + 文档

## 错误码位域

| 位区间 | 长度 | 字段 |
|:---|:---:|:---|
| 63 | 1 | Sign（0 = 错误，1 = 成功） |
| 60-62 | 3 | Reserved（bit0=retryable, bit1=transient） |
| 56-59 | 4 | Level（debug/info/warn/error/fatal） |
| 48-55 | 8 | System（6 大系统域） |
| 32-47 | 16 | Subsystem |
| 16-31 | 16 | Module |
| 0-15 | 16 | Number |

## 快速上手

### 构建错误码

```cpp
#include "error_system.h"
using namespace error_system;

constexpr auto db_err = error_code_t{
    core::error_level_t::fatal,
    domain::system_domain_t::database,
    core::subsystem_id_t{100}, core::module_id_t{200}, core::error_number_t{404}};
```

### 错误上下文与序列化

```cpp
error_context_t ctx{db_err, "数据库连接失败: {}", "timeout"};
ctx.with("host", "192.168.1.100").with("port", 3306);

error_context_serializer_t::to_string(ctx);  // 人类可读
error_context_serializer_t::to_json(ctx);    // JSON
```

### Result 错误传递

```cpp
result_t<int> divide(int a, int b) {
    if (b == 0)
        return result_t<int>::make_error(db_err, "除数不能为零");
    return a / b;
}

auto r = divide(10, 0);
if (!r) std::cerr << error_context_serializer_t::to_string(r.error());
r.value_or(0);  // 失败返回默认值
```

### 注册插件

```cpp
class logger_t : public plugin::i_error_plugin_t {
    std::string_view name() const noexcept override { return "logger"; }
    core::error_level_t min_level() const noexcept override { return core::error_level_t::error; }
    void on_error(const core::error_context_t& ctx) noexcept override {
        std::cerr << error_context_serializer_t::to_string(ctx) << "\n";
    }
};

logger_t logger;
plugin::plugin_registry_t::instance().register_plugin_ref(logger);
```

## 安装

### 环境要求

C++17（GCC 9+ / Clang 10+ / MSVC 19.20+）、CMake 3.15+、Python 3.6+（可选，错误码生成）

### 源码构建

```bash
git clone https://github.com/YIice-cwj/error_system.git && cd error_system
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
ctest --test-dir build --output-on-failure  # 可选
```

### FetchContent 引入

```cmake
include(FetchContent)
FetchContent_Declare(error_system
    GIT_REPOSITORY https://github.com/YIice-cwj/error_system.git
    GIT_TAG v3.0.0)
FetchContent_MakeAvailable(error_system)
target_link_libraries(my_app PRIVATE error_system::error_system)
```

### 最小化构建（嵌入式）

```bash
cmake -S . -B build-min -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DERROR_SYSTEM_ENABLE_STACKTRACE=OFF -DERROR_SYSTEM_ENABLE_LOCATION=OFF \
    -DERROR_SYSTEM_BUILD_TESTS=OFF -DERROR_SYSTEM_BUILD_EXAMPLES=OFF
```

> 完整编译选项见 [架构设计](docs/architecture.md#编译配置)

## 文档

| 文档 | 内容 |
|------|------|
| [Core API](docs/api/core.md) | `error_code_t` `error_context_t` `result_t` `error_registry_t` |
| [Config API](docs/api/config.md) | 特性开关、堆栈/格式化器/i18n 配置 |
| [i18n API](docs/api/i18n.md) | 多语言消息目录与回退 |
| [Mapping API](docs/api/mapping.md) | HTTP / gRPC 状态码映射 |
| [Migration API](docs/api/migration.md) | 错误码废弃与迁移 |
| [Plugin API](docs/api/plugin.md) | 插件接口、注册表、路由、异步通知 |
| [Utils API](docs/api/utils.md) | 字符串、JSON、文件、异步队列、堆栈 |
| [Bridge API](docs/api/bridge.md) | C ABI 导出、std::error_code 桥接 |
| [Async API](docs/api/async.md) | async_result_t 链式异步错误处理 |
| [架构设计](docs/architecture.md) | 分层架构、关键设计决策、编译配置 |
| [决策树](docs/decision_tree.md) | 通知模式、查询路径、序列化等选型指南 |
| [错误码生成](docs/error_code_generation.md) | JSON 配置 → C++ 头文件 + 字典 + 文档 |

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
