# 基准对比：error_system vs 传统错误码

基于 `tests/migration/perf/plain_error_code_benchmark.cc`，对比 plain int / `std::error_code` / error_system 在构造、传播、序列化三个维度的开销。

## 测试环境

- macOS / Apple Silicon (10 核)
- C++17 · `-O2`（Release）/ `-O0 -g`（Debug）
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
| plain int | 0.23 ns | 2.05 ns | 1× |
| std::error_code | 0.23 ns | 4.25 ns | 1× |
| `result_t::make_success` | 0.40 ns | 8.70 ns | 1.7× |

成功路径仅多 0.17 ns（Release），可忽略不计。

### 错误路径

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 1.98 ns | 1× |
| std::error_code | 0.23 ns | 4.24 ns | 1× |
| `result_t::make_error(code, msg)` | 61.8 ns | 598 ns | 270× |
| `result_t::make_error(ctx)` | 61.9 ns | 579 ns | 270× |
| `error_context_t` 直接构造 | 61.0 ns | 531 ns | 266× |

错误路径的 61 ns 主要来自 `runtime_block_t` 堆分配（message + source_location）。这是**有意的权衡**：错误路径频率远低于成功路径，用 61 ns 换取结构化上下文（消息、位置、因果链、负载）。

## 错误传播

### 成功传播

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int (if + return) | 0.23 ns | 2.28 ns | 1× |
| `result_t` (is_error 检查) | 0.40 ns | 27.3 ns | 1.7× |

### 错误传播

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 2.43 ns | 1× |
| `result_t` | 63.7 ns | 679 ns | 277× |

成功传播仅多 0.17 ns。错误传播的 63.7 ns 包含 error_context_t 的拷贝（Move-Only 下为移动，开销来自 runtime_block 引用计数）。

## 错误输出

| 方式 | Release | Debug | 说明 |
|------|--------:|------:|------|
| `snprintf` (plain) | 41.0 ns | 41.7 ns | 手动格式化到 buf |
| `to_string()` | 143 ns | 337 ns | 结构化文本输出 |
| `to_json()` | 156 ns | 316 ns | JSON 输出 |

`to_string`/`to_json` 比 `snprintf` 慢 3-4 倍，但输出的是结构化上下文（错误码位域分解 + 消息 + 负载 + 因果链），而非单行文本。

## Debug vs Release 开销比

| 方式 | Release | Debug | D/R 比 |
|------|--------:|------:|:---:|
| plain int 成功 | 0.23 ns | 2.05 ns | 8.9× |
| std::error_code 成功 | 0.23 ns | 4.25 ns | 18.7× |
| result_t 成功 | 0.40 ns | 8.70 ns | 21.8× |
| result_t 错误 | 61.8 ns | 598 ns | 9.7× |
| result_t 传播成功 | 0.40 ns | 27.3 ns | 68× |
| to_string | 143 ns | 337 ns | 2.4× |

Debug 构建下 `result_t` 成功传播的 27.3 ns 包含 Debug 断言（`checked_` 标志检查）。Release 下编译器优化为 0.40 ns，与 plain int 仅差 0.17 ns。

## 核心结论

### 成功路径：几乎零开销

Release 构建下，`result_t::make_success` 和成功传播比 plain int 仅多 **0.17 ns**。在每秒百万次调用的热路径中，额外开销 < 0.2 ms/s，可忽略。

### 错误路径：有意的权衡

错误路径的 61 ns 换来了 plain int 无法提供的能力：

| 能力 | plain int | error_system |
|------|:---:|:---:|
| 错误消息 | ❌ | ✅ |
| 源位置 | ❌ | ✅ |
| 结构化负载 | ❌ | ✅ |
| 因果链 (wrap) | ❌ | ✅ |
| 堆栈追踪 | ❌ | ✅（可选） |
| 编译期冲突检测 | ❌ | ✅ |
| 强制错误检查 | ❌ | ✅（Debug） |
| i18n 多语言 | ❌ | ✅ |
| HTTP/gRPC 映射 | ❌ | ✅ |
| 插件通知 | ❌ | ✅ |

### Lean 模式：热路径优化

`result_t<T, true>` 仅 24 字节（与 `error_context_t` 相同），在仅需错误码不需要完整上下文的场景下，构造开销与成功路径接近。

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
