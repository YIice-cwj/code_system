# 基准对比：error_system vs 传统错误码

基于 `tests/migration/perf/plain_error_code_benchmark.cc`，对比 plain int / `std::error_code` / error_system 在构造、传播、序列化三个维度的开销。10 次重复取中位数。

## 测试环境

- macOS / Apple Silicon (10 核)
- C++17 · Release `-O3 -DNDEBUG` / Debug `-g -fsanitize=address,undefined`
- Google Benchmark v1.8.3
- 全局初始化关闭 validation/stacktrace/source_location，确保公平对比

## 内存占用

| 类型 | Release | Debug | 说明 |
|------|:---:|:---:|------|
| `int` | 4 B | 4 B | 传统错误码 |
| `std::error_code` | 16 B | 16 B | 标准库（ptr + int + category*） |
| `error_context_t` | **24 B** | **24 B** | v4.0.0 Move-Only（8B code + 8B runtime_block* + 8B cause*） |
| `result_t<int, true>` (Lean) | **16 B** | **48 B** | error_code_t(8B) + T(4B) + 状态(4B) + Debug 增量 32B |
| `result_t<int, false>` (Full) | **32 B** | **64 B** | error_context_t(24B) + T(4B) + 状态(4B) + Debug 增量 32B |

**Debug / Release sizeof 差异**：`result_t` 中 `#ifndef NDEBUG` 包裹 `mutable bool checked_{false}` + `utils::source_location_t created_at_{}`（共 32B：1B checked_ + padding + 24B source_location_t + padding）。Release 完全移除，Debug 保留用于强制错误检查与创建位置追踪。`error_context_t` 不受影响。

## 构造开销

### 成功路径

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 2.06 ns | 1× |
| std::error_code | 0.23 ns | 4.42 ns | 1× |
| `result_t::make_success` | 0.46 ns | 15.3 ns | 2× |

成功路径仅多 0.23 ns（Release），可忽略不计。

### 错误路径

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 2.01 ns | 1× |
| std::error_code | 0.23 ns | 4.36 ns | 1× |
| `result_t::make_error(code, msg)` | 61 ns | 590 ns | 265× |
| `result_t::make_error(ctx)` | 53 ns | 517 ns | 230× |
| `error_context_t` 直接构造 | 52 ns | 466 ns | 226× |

错误路径的 61 ns 主要来自 `runtime_block_t` 堆分配（message + source_location）。这是**有意的权衡**：错误路径频率远低于成功路径，用 61 ns 换取结构化上下文（消息、位置、因果链、负载）。

## 错误传播

### 成功传播

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int (if + return) | 0.23 ns | 2.30 ns | 1× |
| `result_t` (is_error 检查) | 0.24 ns | 31.9 ns | 1.0× |

### 错误传播

| 方式 | Release | Debug | 倍率 |
|------|--------:|------:|:---:|
| plain int | 0.23 ns | 2.44 ns | 1× |
| `result_t` | 63 ns | 658 ns | 274× |

成功传播仅多 0.01 ns。错误传播的 63 ns 包含 error_context_t 的移动（Move-Only 下为指针交换，开销来自 runtime_block 引用计数）。

## 错误输出

| 方式 | Release | Debug | 说明 |
|------|--------:|------:|------|
| `snprintf` (plain) | 36 ns | 37 ns | 手动格式化到 buf |
| `to_string()` | 140 ns | 393 ns | 结构化文本输出 |
| `to_json()` | 144 ns | 316 ns | JSON 输出 |

`to_string`/`to_json` 比 `snprintf` 慢约 4 倍，但输出的是结构化上下文（错误码位域分解 + 消息 + 负载 + 因果链），而非单行文本。

## Lean vs Full 对比

基于 `tests/migration/perf/result_lean_benchmark.cc`，对比 `result_t<T, true>`（Lean）与 `result_t<T, false>`（Full）在各操作上的开销。10 次重复取中位数。

### Release 构建（`-O3 -DNDEBUG`）

| 操作 | Full | Lean | 倍率 |
|------|------:|------:|:---:|
| make_success (int) | 0.70 ns | 0.23 ns | 3.0× |
| make_success (string) | 0.83 ns | 0.83 ns | 1.0× |
| make_error (code+msg) | 64.5 ns | 7.81 ns | 8.3× |
| make_error (from ctx) | 19.2 ns | 7.77 ns | 2.5× |
| access_value | 0.24 ns | 0.24 ns | 1.0× |
| access_error_code | 0.24 ns | 0.23 ns | 1.0× |
| is_success | 0.24 ns | 0.23 ns | 1.0× |
| map (success) | 0.52 ns | 0.24 ns | 2.2× |
| map (error) | 18.0 ns | 0.24 ns | 76× |
| and_then (success) | 1.52 ns | 0.24 ns | 6.4× |

### Debug 构建（`-g -fsanitize=address,undefined`）

| 操作 | Full | Lean | 倍率 |
|------|------:|------:|:---:|
| make_success (int) | 15.2 ns | 15.0 ns | 1.0× |
| make_success (string) | 74.3 ns | 74.4 ns | 1.0× |
| make_error (code+msg) | 602 ns | 55.8 ns | 11× |
| make_error (from ctx) | 290 ns | 46.9 ns | 6.2× |
| access_value | 3.92 ns | 4.06 ns | 1.0× |
| access_error_code | 3.83 ns | 3.22 ns | 1.2× |
| is_success | 3.31 ns | 3.26 ns | 1.0× |
| map (success) | 19.3 ns | 19.6 ns | 1.0× |
| map (error) | 288 ns | 16.2 ns | 18× |
| and_then (success) | 43.7 ns | 42.6 ns | 1.0× |

### 关键结论

- **错误路径差异最大**：Release 下 `make_error(code+msg)` Full 比 Lean 慢 8.3 倍（64.5 ns vs 7.81 ns），`map(error)` 慢 76 倍（18.0 ns vs 0.24 ns）。Full 的错误路径需要构造 `error_context_t`（含 message + source_location 堆分配），Lean 仅存储 8B 的 `error_code_t`。
- **成功路径差异小**：Release 下 `make_success(int)` Full 比 Lean 慢 3 倍（0.70 ns vs 0.23 ns），但绝对值都在 1 ns 内，可忽略。`make_success(string)` 两者持平（SSO 优化）。
- **访问操作无差异**：`access_value`/`access_error_code`/`is_success` 在 Release 下均为 0.23–0.24 ns，Lean 与 Full 持平。
- **Debug 下差异缩小**：由于 Debug 构建额外包含 `checked_` 断言与 `created_at_` 记录（共 32B 增量），Full 与 Lean 在成功路径上几乎持平（15.2 ns vs 15.0 ns）。错误路径 Full 仍比 Lean 慢 11 倍。
- **内存占用**：Release 下 Full 32B / Lean 16B（差 16B）；Debug 下 Full 64B / Lean 48B（差 16B，Debug 增量 32B 来自 `checked_` + `created_at_`）。

### 何时选 Lean

- 每秒百万次调用的热路径（网络包解析、高频缓存查询）
- 嵌入式资源受限环境（16B vs 32B 差异显著）
- 错误本身已足够自描述（简单的 not_found / already_exists）

其余场景应选 Full，保留 message/payload/cause/stack 能力，且可运行时打开栈捕获排查问题。

## Debug vs Release 开销比

| 方式 | Release | Debug | D/R 比 |
|------|--------:|------:|:---:|
| plain int 成功 | 0.23 ns | 2.06 ns | 9× |
| std::error_code 成功 | 0.23 ns | 4.42 ns | 19× |
| result_t 成功 | 0.46 ns | 15.3 ns | 33× |
| result_t 错误 | 61 ns | 590 ns | 10× |
| result_t 传播成功 | 0.24 ns | 31.9 ns | 133× |
| to_string | 140 ns | 393 ns | 2.8× |

Debug 构建下 `result_t` 成功传播的 31.9 ns 包含 Debug 断言（`checked_` 标志检查 + `created_at_` 记录）。Release 下编译器优化为 0.24 ns，与 plain int 仅差 0.01 ns。

## 核心结论

### 成功路径：几乎零开销

Release 构建下，`result_t::make_success` 和成功传播比 plain int 仅多 **0.23 ns / 0.01 ns**。在每秒百万次调用的热路径中，额外开销 < 0.3 ms/s，可忽略。

### 错误路径：有意的权衡

错误路径的 61 ns 换来了 plain int 无法提供的能力：

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

`result_t<T, true>` 在 Release 下仅 16 字节（Lean 错误存储为 `error_code_t` 8B + T + 状态），通知走 `on_code(code)` 路径不构造 `error_context_t`，在仅需错误码不需要完整上下文的场景下，构造开销与成功路径接近。Lean 模式下 `error()` 返回 `make_minimal(code)`（无 file:line），`to_string()` 输出 `[ERR: <raw_code>]`。

### Debug 安全网

Debug 构建下的 `checked_` 断言与 `created_at_` 创建位置追踪在 Release 中被完全优化掉（`#ifndef NDEBUG` 包裹），开发阶段捕获漏检错误并报告精确的未检查位置，生产环境零开销。

## 运行基准

```bash
# Release
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release -DERROR_SYSTEM_BUILD_PERF_TESTS=ON
cmake --build build-release -j$(nproc)
./build-release/tests/perf/plain_error_code_benchmark

# Debug
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DERROR_SYSTEM_BUILD_PERF_TESTS=ON
cmake --build build-debug -j$(nproc)
./build-debug/tests/perf/plain_error_code_benchmark
```

> 其他基准：`error_context_benchmark`（5 种特性场景）、`result_lean_benchmark`（Lean vs Full）、`plugin_benchmark`（插件层全场景）、`i18n_benchmark`（多语言查询）。完整性能数据见 [系统配置指南 · 五、性能数据参考](configuration_guide.md#五性能数据参考)。
