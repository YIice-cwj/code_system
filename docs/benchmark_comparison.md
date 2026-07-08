# 基准对比：error_system vs 传统错误码

基于 `tests/migration/perf/plain_error_code_benchmark.cc`，对比 plain int / `std::error_code` / error_system 在构造、传播、序列化三个维度的开销。10 次重复取中位数。

## 测试环境

- macOS / Apple Silicon (10 核)
- C++17 · Release `-O3 -DNDEBUG` / Debug `-g`
- Google Benchmark v1.8.3
- 全局初始化关闭 validation/stacktrace/source_location，确保公平对比

## 内存占用

| 类型 | sizeof | 说明 |
|------|:---:|------|
| `int` | 4 B | 传统错误码 |
| `std::error_code` | 16 B | 标准库（ptr + int + category*） |
| `error_context_t` | **24 B** | v4.0.0 Move-Only（8B code + 8B runtime_block* + 8B cause*） |
| `result_t<int, true>` (Lean) | **24 B** | error_code_t + T + 状态 |
| `result_t<int, false>` (Full) | **40 B** | error_context_t + T + 状态 |

## 构造开销

### 成功路径

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 5.79 ns | 1× |
| std::error_code | 0.23 ns | 13.7 ns | 1× |
| `result_t::make_success` | 0.35 ns | 25.6 ns | 1.5× |

成功路径仅多 0.12 ns（Release），可忽略不计。

### 错误路径

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 5.51 ns | 1× |
| std::error_code | 0.23 ns | 13.6 ns | 1× |
| `result_t::make_error(code, msg)` | 65 ns | 1552 ns | 282× |
| `result_t::make_error(ctx)` | 57 ns | 1357 ns | 246× |
| `error_context_t` 直接构造 | 55 ns | 1256 ns | 247× |

错误路径的 65 ns 主要来自 `runtime_block_t` 堆分配（message + source_location）。这是**有意的权衡**：错误路径频率远低于成功路径，用 65 ns 换取结构化上下文（消息、位置、因果链、负载）。

## 错误传播

### 成功传播

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int (if + return) | 0.23 ns | 7.86 ns | 1× |
| `result_t` (is_error 检查) | 0.47 ns | 65.5 ns | 2.0× |

### 错误传播

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 6.83 ns | 1× |
| `result_t` | 71 ns | 1656 ns | 304× |

成功传播仅多 0.24 ns。错误传播的 71 ns 包含 error_context_t 的移动（Move-Only 下为指针交换，开销来自 runtime_block 引用计数）。

## 错误输出

| 方式 | Release | Debug | 说明 |
|------|--------:|------:|------|
| `snprintf` (plain) | 37 ns | 109 ns | 手动格式化到 buf |
| `to_string()` | 146 ns | 1209 ns | 结构化文本输出 |
| `to_json()` | 152 ns | 996 ns | JSON 输出 |

`to_string`/`to_json` 比 `snprintf` 慢约 4 倍，但输出的是结构化上下文（错误码位域分解 + 消息 + 负载 + 因果链），而非单行文本。

## Debug vs Release 开销比

| 方式 | Release | Debug | D/R 比 |
|------|--------:|------:|:---:|
| plain int 成功 | 0.23 ns | 5.79 ns | 25× |
| std::error_code 成功 | 0.23 ns | 13.7 ns | 60× |
| result_t 成功 | 0.35 ns | 25.6 ns | 73× |
| result_t 错误 | 65 ns | 1552 ns | 24× |
| result_t 传播成功 | 0.47 ns | 65.5 ns | 139× |
| to_string | 146 ns | 1209 ns | 8.3× |

Debug 构建下 `result_t` 成功传播的 65.5 ns 包含 Debug 断言（`checked_` 标志检查）。Release 下编译器优化为 0.47 ns，与 plain int 仅差 0.24 ns。

## 核心结论

### 成功路径：几乎零开销

Release 构建下，`result_t::make_success` 和成功传播比 plain int 仅多 **0.12 ns / 0.24 ns**。在每秒百万次调用的热路径中，额外开销 < 0.3 ms/s，可忽略。

### 错误路径：有意的权衡

错误路径的 65 ns 换来了 plain int 无法提供的能力：

| 能力 | plain int | error_system |
|------|:---:|:---:|
| 错误消息 | 否 | 是 |
| 源位置 | 否 | 是 |
| 结构化负载 | 否 | 是 |
| 因果链 (wrap) | 否 | 是 |
| 堆栈追踪 | 否 | 是（可选） |
| 编译期冲突检测 | 否 | 是 |
| 强制错误检查 | 否 | 是（Debug） |
| i18n 多语言 | 否 | 是 |
| HTTP/gRPC 映射 | 否 | 是 |
| 插件通知 | 否 | 是 |

### Lean 模式：热路径优化

`result_t<T, true>` 仅 24 字节（与 `error_context_t` 相同），错误存储为 `error_code_t`（8B），通知走 `on_code(code)` 路径不构造 `error_context_t`，在仅需错误码不需要完整上下文的场景下，构造开销与成功路径接近。Lean 模式下 `error()` 返回 `make_minimal(code)`（无 file:line），`to_string()` 输出 `[ERR: <raw_code>]`。

### Debug 安全网

Debug 构建下的 `checked_` 断言在 Release 中被完全优化掉。开发阶段捕获漏检错误，生产环境零开销。

## 运行基准

```bash
# Release
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DERROR_SYSTEM_BUILD_PERF_TESTS=ON
cmake --build build-release -j$(nproc)
./build-release/perf/plain_error_code_benchmark

# Debug
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DERROR_SYSTEM_BUILD_PERF_TESTS=ON
cmake --build build-debug -j$(nproc)
./build-debug/perf/plain_error_code_benchmark
```

> 其他基准：`error_context_benchmark`（5 种特性场景）、`result_lean_benchmark`（Lean vs Full）、`plugin_benchmark`（插件层全场景）、`i18n_benchmark`（多语言查询）。
