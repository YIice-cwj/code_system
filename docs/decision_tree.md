# 决策树

面向使用者的场景化选型指南，覆盖 8 个常见决策点，给出推荐路径与取舍说明。

## 1. 通知模式选择

何时使用 `sync` / `async_queue` / `sync_deferred`。

```
make_error 时立即通知？
├─ 是 → on_error() 可能阻塞？
│       ├─ 否 → sync              ← 日志、指标
│       └─ 是 → 可接受丢失？
│               ├─ 是 → async_queue   ← 网络上报、持久化
│               └─ 否 → sync_deferred ← 事务累积、批处理
└─ 否（显式 flush）→ sync_deferred
```

| 模式 | 调用时机 | 丢失风险 | 适用场景 |
|------|----------|:---:|------|
| `sync` | `make_error` 时同步调用 | 无 | 日志、指标（轻量、非阻塞） |
| `async_queue` | `make_error` 时入队，后台消费 | 满时拒绝 | 网络上报、持久化（可能阻塞） |
| `sync_deferred` | 入缓冲，显式 `flush_deferred_notifications()` | 满时丢弃 | 请求批处理、事务边界累积 |

```cpp
using namespace error_system::config;
feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::async_queue);
plugin::plugin_registry_t::instance().flush_deferred_notifications();  // 请求结束前 flush
```

详见 [Plugin 层 API · plugin_registry_t](api/plugin.md#plugin_registry_t)。

## 2. 错误码元数据查询路径选择

何时使用 `get_info()` vs `get_info_cached()`。

```
热路径？
├─ 否 → 需要强一致？ → get_info()           ← 管理工具、偶尔查询
└─ 是 → 运行时频繁注销？
        ├─ 是 → get_info()           ← 缓存频繁失效反而更慢
        └─ 否 → get_info_cached()    ← 线程本地缓存，命中零锁
```

| 路径 | 锁开销 | 一致性 | 适用场景 |
|------|:---:|:---:|------|
| `get_info()` | `shared_lock` | 强一致 | 管理工具、偶尔查询 |
| `get_info_cached()` | 命中零锁，未命中一次 `shared_lock` | 缓存失效（epoch 版本检测）后最终一致 | error_context_t 构造、校验 |

```cpp
auto& reg = core::error_registry_t::instance();
auto meta = reg.get_info_cached(code);  // 热路径：命中零锁
auto fresh = reg.get_info(code);        // 管理工具：强一致
```

缓存失效（epoch 版本检测）：任何 `register_*` / `unregister_*` 调用 `bump_epoch_()`（release 序），线程本地缓存检测纪元变化后整体失效重建。详见 [Core 层 API · error_registry_t](api/core.md#error_registry_t)。

## 3. 错误码废弃与迁移决策

何时使用 `mark_deprecated()` / `register_migration()` / 单跳 vs 递归迁移。

```
需要下线？
├─ 否 → 需要别名映射？ → register_migration()      ← 仅建立映射，不标记废弃
└─ 是 → 有替代码？
        ├─ 是 → mark_deprecated(code, {reason, replacement, ...})  ← 自动建立 migration
        └─ 否 → mark_deprecated(code, {reason})   ← 仅标记废弃

单跳 vs 递归：
├─ 一次映射 → migrate()       ← a → b（即使 b 也有映射也停止）
└─ 最终码   → migrate_recursive()  ← a → b → c → 终点，环检测最大深度 16
```

```cpp
auto& reg = migration::error_migration_registry_t::instance();
reg.mark_deprecated(ERR_OLD_DB_POOL, {"v2.0 改用 V2", ERR_DB_POOL_V2, "2.0.0", "3.0.0"});
reg.register_migration(ERR_USER_NOT_FOUND_V1, ERR_USER_NOT_FOUND_V2);  // 别名映射
auto terminal = reg.migrate_recursive(ERR_OLD_DB_POOL);  // 递归到终点
```

废弃状态与迁移映射分离存储：`unmark_deprecated()` 不会清除迁移映射，便于先停止废弃警告再逐步下线。详见 [Migration 层 API](api/migration.md#error_migration_registry_t)。

## 4. i18n 消息查询回退路径

`i18n_t::get_message()` 沿 locale parent 链逐级回退，直至 `en_US`（链终点）后返回空。

```
get_message(locale, code)（显式 locale）：
└─ locale → parent(locale) → ... → en_US → 空 string
    （每级查到消息即返回）

get_message(code)（单参，使用 i18n_config_t::resolve_output_locale()）：
└─ resolve_output_locale() 起沿 parent 链回退（同上路径）
    resolve_output_locale() = output_locale 已设置 ? output_locale : default_locale
```

parent 链由 `config::i18n_config_t::get_locale_parent()` 决定，默认按内置 `LOCALE_PARENT_TABLE` 推断（可运行时覆盖）。内置非平凡链仅 `zh_TW → zh_CN`，其余非 `en_US` locale 直接回退 `en_US`。

```cpp
using namespace error_system::i18n;
auto& catalog = i18n_t::instance();
catalog.set_default_locale(locale_t::zh_CN);
catalog.register_message(locale_t::zh_CN, ERR_DB_TIMEOUT, "数据库连接超时");
catalog.register_message(locale_t::en_US, ERR_DB_TIMEOUT, "Database connection timeout");
catalog.set_active_locale(locale_t::zh_TW);
catalog.get_message(ERR_DB_TIMEOUT);                   // zh_TW → parent(zh_CN) → "数据库连接超时"
catalog.get_message(locale_t::fr_FR, ERR_DB_TIMEOUT);  // fr_FR → parent(en_US) → "Database connection timeout"
```

`clear_active_locale()` 表示不用 active locale，`resolve_output_locale()` 回退到 default_locale。详见 [i18n 层 API · i18n_t](api/i18n.md#i18n_t)。

## 5. 序列化格式选择

何时使用 `to_string()` / `to_json()` / `to_binary()`。

```
输出目标？
├─ 人类可读日志 / 终端 → to_string()    ← 含因果链 ↳ Caused by:、堆栈、源位置
├─ 结构化日志 / Web API → to_json()      ← JSON 对象，code 为字符串，含 cause 递归
└─ RPC 跨语言 / 持久化   → to_binary()   ← 紧凑二进制，小端序，magic + version 头，不含栈帧
```

| 格式 | 体积 | 可读性 | 跨语言 | 因果链 | 栈帧 |
|------|:---:|:---:|:---:|:---:|:---:|
| `to_string()` | 大 | 优 | 否 | 含 | 含 |
| `to_json()` | 中 | 良 | 是（JSON） | 含 | 含 |
| `to_binary()` | 小 | 无 | 需解析器 | 含 | 不含 |

```cpp
auto ctx = core::error_context_serializer_t::from_binary(bin);   // 校验 magic + version
auto ctx2 = core::error_context_serializer_t::from_json(json);   // JSON 格式校验
if (!ctx) { /* magic/version 不匹配或数据损坏 */ }
```

详见 [Core 层 API · error_context_serializer_t](api/core.md#error_context_serializer_t)。

## 6. 插件开发模式选择

开发插件时的实现策略选择。

```
通知模式？
├─ sync / sync_deferred（on_error / on_code 在调用线程执行）
│   ├─ 需按码/域/模块路由？ → error_router_plugin_t   ← 框架内置路由（Full + Lean 双路径）
│   ├─ 需去重/采样？        → error_dedup_sampler_t   ← 内置去重+采样
│   └─ 否                   → 自定义插件
└─ async_queue（on_error / on_code 在后台线程执行）
    ├─ 需跨进程/网络转发？ → 自定义插件 + 网络发送（I/O 在后台线程，安全）
    └─ 否                 → 自定义插件
```

```cpp
class my_plugin_t : public i_error_plugin_t {
    core::error_level_t min_level() const noexcept override { return core::error_level_t::error; }
    void on_error(const core::error_context_t& ctx) noexcept override { /* Full 路径：完整上下文 */ }
    void on_code(core::error_code_t code) noexcept override { /* Lean 路径：仅错误码，默认空实现 */ }
    std::string_view name() const noexcept override { return "my_plugin"; }
};
```

Full 路径与 Lean 路径互斥：`result_t<T, false>` 触发 `on_error`，`result_t<T, true>` 触发 `on_code`。仅需 Full 路径时可省略 `on_code` override（默认空实现）。

详见 [Plugin 层 API · 插件开发指南](api/plugin.md#插件开发指南)。

## 7. 错误传递方式选择

何时使用 `result_t<T, Lean>` vs `error_exception_t` vs `async_result_t` vs 直接返回 `error_context_t`。

```
错误传递方式？
├─ 异常不可用 / 性能敏感 → result_t<T, Lean>
│   ├─ 需完整错误上下文 → result_t<T, false>（Full） ← union + result_state_t，链式操作
│   ├─ 热路径仅需错误码 → result_t<T, true>（Lean）  ← 8 字节 error_code_t，on_code 通知，零堆分配
│   └─ 异步链式         → async_result_t              ← then/recover，std::future 包装
├─ 需与异常生态集成     → error_exception_t            ← std::exception 子类
└─ 仅传递错误上下文     → error_context_t              ← 值语义，因果链 wrap
```

| 方式 | 异常开销 | 链式操作 | 异常兼容 | 适用场景 |
|------|:---:|:---:|:---:|------|
| `result_t<T, false>`（Full） | 无 | map/and_then/or_else/match | 否 | 通用错误传递 |
| `result_t<T, true>`（Lean） | 无 | map/and_then/or_else/match（无 context） | 否 | 热路径、仅需错误码 |
| `async_result_t` | 无（回调异常→fatal） | then/recover | 否 | 异步操作链 |
| `error_exception_t` | 有 | 无 | 是 | 跨异常/非异常边界 |
| `error_context_t` | 无 | wrap（因果链） | 否 | 简单传递 |

Lean 模式（`result_t<T, true>`）错误存储为 `error_code_t`（8B），`error()` 返回 `make_minimal(code)`（无 file:line），`to_string()` 输出 `[ERR: <raw_code>]`；通知走 `on_code(code)` 路径，不构造 `error_context_t`。

注意：Debug 构建下 `result_t` 析构时若错误未被检查（is_success/is_error/value/error 等），触发 assert 防止吞错误；Release 构建零开销。

详见 [Core 层 API · result_t](api/core.md#result_tt)、[Async 层 API](api/async.md)、[error_exception_t](api/core.md#error_exception_t)。

## 8. HTTP/gRPC 状态码映射选择

何时使用 `status_mapper_t`。

```
需要映射到 HTTP/gRPC 状态码？
├─ 是 → status_mapper_t::to_http_status(code) / to_grpc_status(code)
│        ← constexpr 内置默认映射（按错误等级 + 系统域）
└─ 否 → 不调用
```

`status_mapper_t` 为纯静态工具类（不可实例化），所有方法 constexpr，不支持运行时自定义映射；如需特殊规则，调用方自行判断后用 `http_status_t::from_int()` / `grpc_status_t::from_int()` 构造。

| 错误等级 | 系统域 | HTTP | gRPC |
|------|------|:---:|:---:|
| debug / info / warn | * | 200 OK | OK |
| error（retryable / transient） | * | 503 Service Unavailable | UNAVAILABLE |
| error | application | 400 Bad Request | INVALID_ARGUMENT |
| error | database | 503 Service Unavailable | DATA_LOSS |
| error | middleware | 503 Service Unavailable | UNAVAILABLE |
| error | third_party | 502 Bad Gateway | UNAVAILABLE |
| error | system / none | 500 Internal Server Error | INTERNAL |
| fatal | * | 500 Internal Server Error | INTERNAL |

```cpp
using namespace error_system::mapping;
http_status_t http = status_mapper_t::to_http_status(ERR_DB_TIMEOUT);   // 503
grpc_status_t grpc = status_mapper_t::to_grpc_status(ERR_DB_TIMEOUT);  // UNAVAILABLE
http.to_int();  // 503
```

retryable / transient 标志在所有等级上优先映射为 503 / UNAVAILABLE，提示客户端重试。详见 [Mapping 层 API](api/mapping.md#status_mapper_t)。

## 快速索引

| 决策点 | 章节 |
|------|------|
| 通知模式（sync/async/deferred） | [1](#1-通知模式选择) |
| 元数据查询（cached vs direct） | [2](#2-错误码元数据查询路径选择) |
| 废弃/迁移（mark/migrate/recursive） | [3](#3-错误码废弃与迁移决策) |
| i18n locale 回退 | [4](#4-i18n-消息查询回退路径) |
| 序列化格式（string/json/binary） | [5](#5-序列化格式选择) |
| 插件开发模式 | [6](#6-插件开发模式选择) |
| 错误传递方式（result/exception/context） | [7](#7-错误传递方式选择) |
| HTTP/gRPC 状态映射 | [8](#8-httpgrpc-状态码映射选择) |
