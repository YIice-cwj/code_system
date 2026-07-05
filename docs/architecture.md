# 架构设计

## 分层架构

| 层 | 核心组件 |
|------|------|
| Config | `feature_flags_t` `stacktrace_config_t` `formatter_config_t` `i18n_config_t` |
| i18n | `i18n_t` `locale_t` `subsystem_module_catalog_t` |
| Mapping | `status_mapper_t` `http_status_t` `grpc_status_t` |
| Migration | `error_migration_registry_t` |
| Core | `error_code_t` `error_context_t` `result_t<T>` `error_registry_t` `error_builder_t` `error_exception_t` `error_context_serializer_t` `error_context_initializer_t` |
| Domain | 6 大系统域枚举 |
| Plugin | `i_error_plugin_t` `plugin_registry_t` `error_router_plugin_t` `async_notification_channel_t` `error_dedup_sampler_t` `log_plugin_t` `metric_plugin_t` |
| Utils | `async_queue_t` `string_utils_t` `string_format_t` `json_dict_t` `json_lexer_t` `file_utils_t` `stack_trace_utils_t` `source_location_t` |
| Bridge | `c_abi_export` `std_error_code_bridge` `error_system_category_t` |
| Async | `async_result_t` |

核心原则：下层不依赖上层，`error_code_t` 不感知 Plugin。原 `error_config_t` 与 `error_context_t` 已按 SRP 拆分，`error_config.h` 仅作向后兼容入口，新代码直接包含细分头文件。

## 各层职责

| 层 | 职责 |
|------|------|
| Config | 编译期特性开关 + per-code 覆盖 + 通知模式 + i18n 配置，原子无锁访问。 |
| i18n | 多语言消息目录与子系统/模块名称两级映射，locale 回退查询。 |
| Mapping | 错误码到 HTTP/gRPC 状态码的 constexpr 纯函数映射。 |
| Migration | 错误码废弃标记与迁移注册，单跳或递归迁移（环检测，最大深度 16）。 |
| Core | 64 位错误码、错误上下文、序列化、注册表、Result 与异常封装。 |
| Domain | 6 大系统域枚举（none/system/middleware/database/application/third_party）。 |
| Plugin | 插件接口、RCU 无锁注册表、路由分发、异步通知通道、去重采样、日志插件、指标插件。 |
| Utils | 异步队列、字符串处理、JSON 解析、文件操作、堆栈跟踪、源位置封装。 |
| Bridge | C ABI 句柄导出、std::error_code 双向桥接。 |
| Async | 基于 std::future 的 then/recover 链式异步错误处理。 |

## 关键设计决策

1. **64 位位移 + 掩码**：`LEVEL_SHIFT=56` 单条指令完成字段提取，100% 避免位域未定义行为；Reserved 3 bits 作语义位（retryable/transient/预留）。
2. **constexpr 全链路**：`error_code_t` 构造、`to_string()`、`from_string()`、`hash()` 均为 constexpr，编译期确定值进入只读数据段。
3. **循环依赖打破**：循环 1（plugin_registry ↔ error_context）将 `initialize()` 放 `.cc` 安全 include；循环 2（channel ↔ registry）构造时注入回调（依赖倒置）。
4. **简化构造 API（v2.0）**：移除 `code_with_location_t`，引入 `located_code_t` 通过隐式转换从 `error_code_t` 构造时自动捕获调用者位置。
5. **多类型 payload**：`with()` 模板用 `if constexpr` 分派（bool→true/false、算术→to_string、其他→static_cast<string>），字符串优先匹配非模板重载。
6. **全局单例注册表**：Meyer's singleton + `shared_mutex`；`register_plugin(unique_ptr)` 接管所有权，`register_plugin_ref()` 非持有引用，回调在锁外执行。
7. **通知模式**：sync（构造时同步，无丢失）/ async_queue（入队后台消费，满时拒绝）/ sync_deferred（显式 flush，满时丢弃），详见 [决策树 · 1](decision_tree.md#1-通知模式选择)。
8. **result_t<T> 零异常**：`variant` + `get_if` + `thread_local` 哨兵防跨线程污染；`map`/`and_then`/`or_else`/`match` noexcept（match 除外）；拷贝赋值删除，移动赋值 default。
9. **子系统索引（v2.1）**：`subsystem_index_` 将 `get_errors_by_subsystem` 从 O(n) 优化为 O(1) 索引 + O(k) 遍历。
10. **RCU 快照（v2.3）**：`plugin_registry_t` 用 `shared_ptr<const vector>` + `atomic_load/store` 无锁热路径，读者共享所有权避免 use-after-free。
11. **SSO 小负载（v2.3）**：`error_context_t` 载荷 `array<pair,4>` 栈存储 + `unique_ptr<unordered_map>` 惰性溢出，≤4 项零堆分配。
12. **异步队列模板**：`async_queue_t` 死锁安全析构、背压控制、异常隔离、零 `std::function` 开销。
13. **自动堆栈 + Per-Code 覆盖**：优先级 `is_stacktrace_enabled → per-code 覆盖 → 全局阈值`。
14. **安全性设计**：bad_alloc 捕获入 stderr；result 哨兵 thread_local；RCU 共享所有权；队列满背压；持锁回调复制后释放锁；文件 64MB 上限；JSON 代理对丢弃。
15. **SRP 拆分**：原 `error_config_t` / `error_context_t` 按单一职责拆分，`error_config.h` 仅向后兼容入口。
16. **编译期特性开关 + DCE**：`feature_flags_t` 用 `if constexpr` + 编译期常量消除未启用分支运行时开销。

```cpp
[[nodiscard]] static bool is_stacktrace_enabled() noexcept {
    if constexpr (STACKTRACE_ENABLED) { return get_flag_stacktrace_().load(); }
    else { return false; }  // 编译期返回 false，整条分支被 DCE
}
```

17. **错误码自动注册**：`DEFINE_ERROR_CODE` 宏 + `inline const registrar` 静态初始化在 `main()` 前完成注册，避免 SIOF，宏签名详见 [Core 层 API · DEFINE_ERROR_CODE](api/core.md#define_error_code)。
18. **缓存失效（epoch 版本检测）**：`error_registry_t` 用 `atomic<uint64_t> epoch_counter_` 驱动 `thread_local` 环形缓冲（容量 16）失效，register/unregister 调 `fetch_add(1,release)`，读取用 `acquire` 配对，命中零锁开销。

```cpp
void bump_epoch_() noexcept { epoch_counter_.fetch_add(1, std::memory_order_release); }
[[nodiscard]] uint64_t get_epoch() const noexcept { return epoch_counter_.load(std::memory_order_acquire); }
```

19. **i18n 多语言设计**：`i18n_t` 两级哈希 `locale → (code identity → message)`，查询路径 active → default → 空；`subsystem_module_catalog_t` 独立存储避免双源不同步。
20. **错误码迁移与废弃**：`error_migration_registry_t` 分离废弃与迁移两正交维度，`migrate_recursive` 最大深度 16 防栈溢出，环检测后返回当前码。
21. **constexpr 状态码映射**：`status_mapper_t` 纯函数，retryable/transient 优先映射 503/UNAVAILABLE，详见下表。

| 码特征 | HTTP | gRPC |
|------|------|------|
| success | 200 OK | OK |
| retryable / transient | 503 | UNAVAILABLE |
| info / warn | 200 | OK |
| error·application | 400 | INVALID_ARGUMENT |
| error·third_party | 502 | UNAVAILABLE |
| error·database / middleware | 503 | DATA_LOSS / UNAVAILABLE |
| error·system / none | 500 | INTERNAL |
| fatal | 500 | INTERNAL |

## 编译配置

| CMake 选项 | 说明 | 默认 |
|------|------|:---:|
| `ERROR_SYSTEM_ENABLE_STACKTRACE` | 编译堆栈追踪功能 | ON |
| `ERROR_SYSTEM_ENABLE_VALIDATION` | 编译严格错误码验证器 | ON |
| `ERROR_SYSTEM_ENABLE_LOCATION` | 编译源位置宏 | ON |
| `ERROR_SYSTEM_ENABLE_LTO` | 启用 LTO/IPO | OFF |
| `ERROR_SYSTEM_ENABLE_PGO_GENERATE` | PGO 生成阶段 | OFF |
| `ERROR_SYSTEM_ENABLE_PGO_USE` | PGO 使用阶段 | OFF |
| `ERROR_SYSTEM_ENABLE_ASAN` | 地址安全检查器 | OFF |
| `ERROR_SYSTEM_ENABLE_UBSAN` | 未定义行为检测器 | OFF |
| `ERROR_SYSTEM_WARNINGS_AS_ERRORS` | 警告视为错误 | ON |
| `ERROR_SYSTEM_BUILD_TESTS` | 构建单元测试 | ON |
| `ERROR_SYSTEM_BUILD_EXAMPLES` | 构建示例 | ON |
| `ERROR_SYSTEM_BUILD_PERF_TESTS` | 构建性能基准 | ON |

`PGO_GENERATE` 与 `PGO_USE` 不可同时启用。警告 flag：`-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Wdouble-promotion -Wnull-dereference -Wformat=2 -Wnon-virtual-dtor -Wunused -Wunused-parameter -Wunused-variable -Wunused-function`（MSVC `/W4`）。

## 测试架构

框架为 GoogleTest v1.14.0（`FetchContent`）+ `gtest_discover_tests` 注册到 CTest；单元测试镜像 `include/` 结构，链接 `error_system::error_system` + `gtest_main`，仅应用警告选项不应用 LTO/PGO/Sanitizer；性能基准 `tests/perf/` 含 4 个基准文件（Google Benchmark v1.8.3）；代码生成由 Python3 从 `config/errors/*.json` 产出头文件、O(1) 字典与文档。

> **Debug vs Release 测试数量差异**：5 个 death test（`type_safety_test.cc` 中 2 个、`result_unchecked_test.cc` 中 3 个）使用 `EXPECT_DEATH` 断言，受 `#ifndef NDEBUG` 保护。Debug 构建（`NDEBUG` 未定义）编译 666 个用例，Release 构建（`NDEBUG` 定义）编译 661 个用例。两者都是正确的，文档以 Debug 全量值为准。

| 模块 | 文件数 | 用例数 |
|------|:---:|:---:|
| Core | 13 | 260（Debug 全量） |
| Plugin | 7 | 89 |
| Utils | 5 | 137 |
| Config | 2 | 27 |
| Domain | 1 | 12 |
| i18n | 2 | 43 |
| Mapping | 1 | 27 |
| Migration | 1 | 32 |
| Async | 1 | 18 |
| Bridge | 2 | 21 |
| **总计** | **35** | **666**（Debug）/ **661**（Release） |
