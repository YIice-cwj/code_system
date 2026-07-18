# 系统配置指南

错误系统在生命周期不同阶段（开发、调试、压测、生产、排障）有不同诉求。本指南把所有运行时可调项汇总到一处，给出按场景的推荐配置、性能数据参考、决策树与最佳实践。

API 细节请查阅 [Config 层 API](api/config.md)；通知模式选择细节请查阅 [决策树 · 1](decision_tree.md#1-通知模式选择)。

---

## 一、配置项总览

所有运行时配置均为静态、线程安全（`std::atomic` 或 `std::shared_mutex`），无需实例化配置类即可调用。

| 配置类 | 配置项 | 默认值 | 影响范围 |
|--------|--------|--------|---------|
| `feature_flags_t` | `enable_stacktrace` | `true` | 是否在 `error_context_t` 构造时捕获栈帧 |
| `feature_flags_t` | `enable_validation` | `true` | 是否校验 `error_code_t` 字段合法性 |
| `feature_flags_t` | `enable_source_location` | `true` | 是否记录 `file:line:function` 源位置 |
| `feature_flags_t` | `enable_short_filename` | `true` | 是否把源文件路径缩为 basename |
| `feature_flags_t` | `notify_mode` | `sync` | 插件通知派发方式（sync/async_queue/sync_deferred） |
| `stacktrace_config_t` | `min_stacktrace_level` | `error` | 自动捕获栈的最低错误等级 |
| `stacktrace_config_t` | `per_code_stacktrace_level` | 无 | 单个错误码的栈等级覆盖 |
| `i18n_config_t` | `enable_i18n` | `true` | 序列化输出本地化文本还是原始 ID |
| `i18n_config_t` | `default_locale` | `zh_CN` | 未显式指定 locale 时的回退值 |
| `i18n_config_t` | `output_locale` | 未设置 | 运行时切换输出语言 |
| `i18n_config_t` | `locale_parent` | 内置表 | locale 父链覆盖（zh_TW→zh_CN→en_US 等） |
| `formatter_config_t` | `custom_formatter` | `nullptr` | 自定义 `to_string` 文本格式 |
| `plugin_registry_t` | `max_queue_size` | `0`（无限） | 异步通知队列容量上限（背压） |
| `plugin_registry_t` | `deferred_buffer_size` | `1024` | 线程本地延迟通知缓冲容量 |

编译期宏（`-DERROR_SYSTEM_ENABLE_STACKTRACE` / `-DERROR_SYSTEM_ENABLE_VALIDATION` / `-DERROR_SYSTEM_ENABLE_LOCATION`）通过 CMake 控制，决定 `feature_flags_t::STACKTRACE_ENABLED` 等 `constexpr` 值。编译期关闭后，对应的 `set_*` 调用变为无操作，`is_*_enabled()` 恒为 `false`，编译器死代码消除未启用分支。编译期宏细节见 [架构设计 · 编译配置](architecture.md#编译配置)。

---

## 二、Lean vs Full 模式选择

`result_t<T, bool Lean>` 的 `Lean` 是**编译期模板参数**，决定了错误路径的存储类型，**不能运行时切换**。

| 维度 | `result_t<T, true>`（Lean） | `result_t<T, false>`（Full） |
|------|----------------------------|-----------------------------|
| 错误存储 | `error_code_t`（8 字节） | `error_context_t`（24 字节，Move-Only） |
| `result_t<int>` 整体大小（Release） | 16 字节 | 32 字节 |
| `result_t<int>` 整体大小（Debug） | 48 字节 | 64 字节 |
| 构造开销 | 7.8 ns | 73 ns（baseline）/ 170 ns（带栈） |
| 携带信息 | 仅错误码 | code + message + payload + cause + stack |
| 通知路径 | `on_code(code)`，不构造 `error_context_t` | `on_error(ctx)`，完整上下文 |
| 可序列化 | 否 | 是（JSON / binary） |
| `error()` 返回 | `error_context_t::make_minimal(code)`（无 file:line） | `const error_context_t&` |
| `to_string()` | `[ERR: <raw_code>]` | 完整文本（含因果链/堆栈/源位置） |
| `context()` / 可变 `error()` | 否（SFINAE 禁用） | 是 |
| 跨进程传输 | 否（需显式升级） | 是（原生支持） |

### 选择决策

```
是否每秒百万次调用 / 嵌入式受限？
├─ 是 → Lean
└─ 否 → 是否需要跨进程传输 / 用户可见消息 / 因果链调试？
        ├─ 是 → Full
        └─ 否 → Full + 关闭可选特性（性能接近 Lean，保留扩展能力）
```

### Lean 升级到 Full

Lean 模式需要调试时，可显式升级为 Full 上下文：

```cpp
result_t<int, true> lean = some_hot_path();
if (lean.is_error()) {
    error_context_t full{located_code_t{lean.error_code()}, ""};
    log_error(full.to_string());
}
```

### 关键判断

**绝大多数场景应该用 Full**。Full + `feature_flags` 关闭 stacktrace/source_location/validation 后构造只要 73 ns，比 Lean 慢约 8 倍，但保留了 message/payload/cause 能力，且可运行时打开栈捕获排查问题。Lean 仅在确认的性能瓶颈点使用。

---

## 三、运行时配置切换策略

### 推荐做法：Full 模式 + feature_flags 运行时控制

代码始终用 `result_t<T, false>`，通过 `feature_flags_t` 在不同环境切换可选特性。这样 API 统一、类型一致、单元测试可覆盖两种配置、出 bug 时可临时打开 stacktrace 排查。

```cpp
namespace es = error_system;

void configure_for_debug() {
    es::config::feature_flags_t::set_enable_stacktrace(true);
    es::config::feature_flags_t::set_enable_source_location(true);
    es::config::feature_flags_t::set_enable_validation(true);
    es::config::stacktrace_config_t::set_stacktrace_level(es::core::error_level_t::debug);
    es::config::i18n_config_t::set_enable_i18n(true);
    es::config::i18n_config_t::set_default_locale(es::i18n::locale_t::zh_CN);
}

void configure_for_release() {
    es::config::feature_flags_t::set_enable_stacktrace(false);
    es::config::feature_flags_t::set_enable_source_location(false);
    es::config::feature_flags_t::set_enable_validation(false);
    es::config::feature_flags_t::set_notify_mode(es::config::feature_flags_t::notify_mode_t::sync_deferred);
    es::config::i18n_config_t::set_enable_i18n(true);
}
```

### 不推荐：编译期宏切换 Lean/Full

通过 `#ifdef` 在 Release 编译为 Lean、Debug 编译为 Full 的做法有三个问题：

1. **调试体验差**：Release 出 bug 时没有 stacktrace，只能重新编译 Debug 复现
2. **测试覆盖断层**：单元测试通常在 Debug 跑，无法覆盖 Release 的 Lean 路径
3. **ABI 不兼容**：动态库不能跨模式链接，第三方集成困难

### 何时才用真正的 Lean 模式

显式指定 `result_t<T, true>` 的场景：

- 每秒百万次调用的热路径（网络包解析、高频缓存查询）
- 嵌入式资源受限环境（16B vs 32B 差异显著）
- 错误本身已足够自描述（简单的 not_found / already_exists）

---

## 四、典型场景配置

### 1. 开发 / 调试环境

目标：最大化可观测性，性能不敏感。

```cpp
es::config::feature_flags_t::set_enable_stacktrace(true);
es::config::feature_flags_t::set_enable_source_location(true);
es::config::feature_flags_t::set_enable_validation(true);
es::config::stacktrace_config_t::set_stacktrace_level(es::core::error_level_t::debug);
es::config::feature_flags_t::set_notify_mode(es::config::feature_flags_t::notify_mode_t::sync);
```

- 栈捕获阈值降到 `debug`，所有错误都带栈
- 同步通知，插件回调即时可见
- 验证开启，及早发现错误码字段误用

### 2. 生产环境

目标：低开销 + 可观测性平衡。

```cpp
es::config::feature_flags_t::set_enable_stacktrace(false);
es::config::feature_flags_t::set_enable_source_location(false);
es::config::feature_flags_t::set_enable_validation(false);
es::config::feature_flags_t::set_notify_mode(es::config::feature_flags_t::notify_mode_t::sync_deferred);
es::config::i18n_config_t::set_enable_i18n(true);
es::config::i18n_config_t::set_default_locale(es::i18n::locale_t::zh_CN);
```

- 关闭栈捕获 / 源位置 / 验证，构造降到 73 ns
- 用 `sync_deferred` 批量通知，单次锁、批量派发
- 保留 i18n 输出本地化消息

### 3. 高频热路径

目标：极致性能。

```cpp
es::config::feature_flags_t::set_enable_stacktrace(false);
es::config::feature_flags_t::set_enable_source_location(false);
es::config::feature_flags_t::set_enable_validation(false);
es::config::i18n_config_t::set_enable_i18n(false);  // 输出原始 ID 数字
```

- 全部可选特性关闭，构造 73 ns，吞吐 13.7 M/s
- i18n 关闭，序列化输出数字 ID 省去字符串查找
- 若仍不够快，热路径函数显式返回 `result_t<T, true>`（Lean），构造降到 7.8 ns

### 4. 排障模式（生产临时开启）

目标：复现问题时获取完整上下文，排障后恢复。

```cpp
// 排障开启
es::config::feature_flags_t::set_enable_stacktrace(true);
es::config::stacktrace_config_t::set_stacktrace_level(es::core::error_level_t::warn);

// 排障结束
es::config::feature_flags_t::set_enable_stacktrace(false);
```

- 仅对 `warn` 及以上错误捕获栈，避免 `debug/info` 噪声
- 无需重新编译，运行时即时生效
- 排障完成后关闭即可

### 5. 差异化堆栈

目标：只对关键错误捕获栈，普通错误不捕获。

```cpp
es::config::feature_flags_t::set_enable_stacktrace(true);
es::config::stacktrace_config_t::set_stacktrace_level(es::core::error_level_t::error);
es::config::stacktrace_config_t::set_per_code_stacktrace_level(
    ERR_DB_CONNECTION_LOST.get_identity_code(),
    es::core::error_level_t::debug);  // 数据库连接丢失即使 info 级也捕获栈
```

- 全局阈值 `error`，普通错误不捕栈
- 关键错误码单独降到 `debug`，任何级别都捕栈

### 6. 请求处理批处理

目标：请求期间累积错误，请求结束批量通知。

```cpp
es::config::feature_flags_t::set_notify_mode(es::config::feature_flags_t::notify_mode_t::sync_deferred);

void handle_request() {
    // 请求处理期间构造 error_context_t 自动入线程本地缓冲
    process();
    // 请求结束统一 flush
    es::plugin::plugin_registry_t::instance().flush_deferred_notifications();
}
```

- 单次锁、批量派发，`sync_deferred` 入缓冲 53 ns vs `sync` 即时派发 52 ns；批量 flush 时摊薄优势显著
- 缓冲满（默认 1024）会丢弃并设置溢出标志
- 异常退出时用 `clear_deferred_notifications()` 丢弃累积通知

### 7. 跨进程 / 跨语言传输

目标：完整上下文跨进程传递。

```cpp
// 发送方
es::core::error_context_t ctx = full_result.error();
std::vector<uint8_t> binary = ctx.to_binary();

// 接收方
auto recovered = es::core::error_context_t::from_binary(binary);
// cause 链、payload、location 均还原（binary 不含栈帧）
```

- 用 binary 序列化（56 ns），比 JSON（3608 ns）快 64×
- Lean 模式不支持序列化，跨进程必须用 Full

### 8. 错误洪水治理

目标：高并发下抑制重复错误，降低下游压力。

```cpp
es::plugin::error_dedup_sampler_t sampler;
sampler.set_dedup_window_ms(1000);  // 同一 identity 1s 内只放行首次
sampler.set_sample_rate(0.1);       // 放行 10%

if (sampler.should_be_forwarded(ctx)) {
    notifier.notify(ctx);
}
```

- 去重 + 采样组合，6.87 ns 判定
- 适合告警通道、日志通道等下游易被洪水冲垮的场景

---

## 五、性能数据参考

以下数据基于基准测试（Release 构建 `-O3 -DNDEBUG`，10 核 CPU，10 次重复取中位数），用于配置决策参考。Debug 数据见 [基准对比](benchmark_comparison.md)。

### 错误上下文构造开销

| 配置 | 构造耗时 | 吞吐 | 内存 |
|------|---------|------|------|
| Full + 全开（trace+loc+plugin） | 170 ns | 5.89 M/s | 24B |
| Full + plugin only | 76.4 ns | 13.09 M/s | 24B |
| Full + baseline（全关） | 73.1 ns | 13.68 M/s | 24B |
| Lean | 7.8 ns | 128.2 M/s | 16B |

### 序列化开销（Full 模式）

| 操作 | baseline | full（带栈） | 倍数 |
|------|---------|-------------|------|
| `to_binary()` | 55.9 ns | 83.8 ns | 1.5× |
| `from_binary()` | 44.6 ns | 68.6 ns | 1.5× |
| `to_json()` | 198 ns | 3608 ns | 18× |
| `from_json()` | 298 ns | 3818 ns | 13× |
| `to_string()` | 144 ns | 1003 ns | 7.0× |

**关键结论**：`to_binary`/`from_binary` 不序列化栈帧（baseline 与 full 列的差异来自带栈对象的内存布局间接影响，非栈帧处理开销）；`to_json`/`to_string` 受栈帧影响显著（198 ns vs 3608 ns）。跨进程传输首选 binary——栈帧信息丢失是已知权衡，换取稳定且紧凑的体积。

### 拷贝 / 移动开销

| 操作 | baseline | full（带栈） |
|------|---------|-------------|
| 拷贝构造 | 18.5 ns | 18.6 ns |
| 移动构造 | 77.4 ns | 167 ns |

**关键结论**：带栈时移动构造升至 167 ns（shared_ptr 原子计数 + 栈帧共享）。高频移动场景可关闭栈或用 Lean。

### 通知模式开销（单错误，无插件回调）

| 模式 | 耗时 | 说明 |
|------|------|------|
| `sync` | 51.9 ns | 即时派发 |
| `sync_deferred` | 52.6 ns | 入线程本地缓冲 |
| `async_queue` | 52.3 ns | 入队 + 唤醒工作线程 |

### 通知模式 × 真实插件矩阵

| 模式 \ 插件 | log | metric | router |
|-------------|-----|--------|--------|
| sync | 69.4 ns | 69.6 ns | 69.5 ns |
| async_queue | 70.1 ns | 70.1 ns | 70.0 ns |
| sync_deferred | 69.4 ns | 69.4 ns | 69.4 ns |

**关键结论**：三种通知模式在单错误 + 单插件场景下开销接近（69-71 ns），差异在工作线程是否参与：`async_queue` 仅入队，实际回调由后台线程异步执行；`sync` / `sync_deferred` 在当前线程同步执行回调。`sync_deferred` 的批量摊薄优势在 `flush_deferred_notifications()` 时体现——单次锁、遍历全部缓冲。

### 单插件 on_error 回调开销

| 插件 | 耗时 | 说明 |
|------|------|------|
| log_plugin | 169 ns | 含格式化输出 |
| metric_plugin | 13.9 ns | 仅计数器递增 |
| router_plugin | 19.6 ns | O(1) 哈希查表 |

### 插件组合开销

| 组合 | 耗时 |
|------|------|
| log + metric | 189 ns |
| log + metric + router | 196 ns |
| log + metric + router + dedup | 231 ns |

### 插件注册 / 注销开销

| 插件数 | register | unregister |
|--------|----------|------------|
| 1 | 212 ns | 226 ns |
| 4 | 742 ns | 995 ns |
| 16 | 3524 ns | 4991 ns |
| 64 | 21788 ns | 31740 ns |

O(n) 复杂度（写锁 + 快照拷贝），冷启动可接受。

### 去重采样器开销

| 场景 | 耗时 | 说明 |
|------|------|------|
| 全放行（无去重） | 19.6 ns | rate=1.0 |
| 仅去重窗口 | 21.8 ns | dedup=1day, rate=1.0 |
| 仅采样 | 6.60 ns | rate=0.1 |
| 去重 + 采样 | 6.87 ns | dedup=1day + rate=0.1 |

### i18n 查询开销

| 场景 | 耗时 | 说明 |
|------|------|------|
| register_single | 14.2 ns | 注册单条消息 |
| get_message_hit | 10.4 ns | 直接命中 |
| get_message_fallback | 9.34 ns | locale parent 链回退命中 |
| get_message_miss | 12.0 ns | 未命中（遍历完整 parent 链） |
| locale_conversion | 1.17 ns | locale_t 转换 |

### Result Lean vs Full 对比

| 操作 | Full | Lean | 倍率 |
|------|------|------|:---:|
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

**关键结论**：Lean 模式在错误路径上比 Full 快约 8 倍（7.8 ns vs 64.5 ns），map_error 快 76 倍（0.24 ns vs 18.0 ns），适合每秒百万次调用的热路径。

---

## 六、配置最佳实践

### 1. 初始化时序

`plugin_registry_t::instance()` 首次调用时自注册为默认通知器。必须在构造任何 `error_context_t` 之前完成初始化，否则错误通知会被静默丢弃。

```cpp
int main() {
    // 1. 先配置特性开关
    es::config::feature_flags_t::set_enable_stacktrace(false);
    // 2. 再访问 plugin_registry（触发单例构造 + 自注册）
    auto& registry = es::plugin::plugin_registry_t::instance();
    // 3. 注册业务插件
    registry.register_plugin(std::make_unique<es::plugin::log_plugin_t>(...));
    // 4. 业务代码可以构造 error_context_t
    run_business();
}
```

### 2. 配置变更可见性

所有 `feature_flags_t::set_*` 通过 `std::atomic` 存储，对其他线程立即可见。但**已在执行中的 `error_context_t` 构造不会受影响**——配置读取发生在构造瞬间。

### 3. 线程安全约束

- `feature_flags_t` / `i18n_config_t` / `stacktrace_config_t`：全静态 + atomic，任意线程任意时刻可读可写
- `formatter_config_t::set_custom_formatter`：内部加锁，但回调本身的线程安全由调用方保证
- `plugin_registry_t::flush_deferred_notifications`：仅操作当前线程的缓冲，必须由累积通知的同一线程调用
- `i18n_t::register_message` / `get_message`：内部 `shared_mutex`，读多写少场景无锁

### 4. 资源释放

- `plugin_registry_t` 单例生命周期与进程一致，析构时自动释放所有 owned 插件
- `async_queue` 模式下的后台工作线程由 `plugin_registry_t` 管理，无需手动 join
- `sync_deferred` 模式下的线程本地缓冲在线程退出时自动释放

### 5. 测试隔离

单元测试中务必在 SetUp/TearDown 重置配置，避免用例间污染：

```cpp
void SetUp() override {
    es::config::feature_flags_t::set_enable_validation(false);
    es::config::feature_flags_t::set_enable_stacktrace(false);
    es::config::feature_flags_t::set_enable_source_location(false);
    es::config::i18n_config_t::reset_all_locale_parents();
    es::plugin::plugin_registry_t::instance().clear();
}

void TearDown() override {
    es::plugin::plugin_registry_t::instance().clear();
    es::core::error_registry_t::instance().unregister_all();
    es::config::i18n_config_t::reset_all_locale_parents();
}
```

### 6. 配置切换的代价

| 切换项 | 切换代价 | 备注 |
|--------|---------|------|
| `set_enable_stacktrace` | atomic store，~1 ns | 仅影响后续构造的 context |
| `set_notify_mode` | atomic store，~1 ns | 队列 / 缓冲状态保留 |
| `set_stacktrace_level` | atomic store，~1 ns | 立即生效 |
| `set_custom_formatter` | 加锁 + function 拷贝，~50 ns | 建议启动时设置一次 |
| `set_locale_parent` | atomic store 数组，~5 ns | 影响 i18n 查询路径 |

所有配置切换都是纳秒级，可在运行时频繁切换（如根据请求来源动态调整栈捕获等级）。

---

## 七、配置决策树

```
需要跨进程传输 / 用户可见消息 / 因果链调试？
├─ 是 → Full 模式
│       └─ 是否每秒百万次调用？
│           ├─ 是 → 关闭 stacktrace/source_location/validation（73 ns）
│           └─ 否 → 按场景开启可选特性
└─ 否 → 是否每秒百万次调用 / 嵌入式受限？
        ├─ 是 → Lean 模式（7.8 ns，16B）
        └─ 否 → Full + 关闭可选特性（保留扩展能力）

通知模式选择：
├─ 请求处理批处理 → sync_deferred（53 ns 入缓冲，批量 flush 摊薄）
├─ 普通业务 → sync（52 ns，即时可见）
└─ 不能阻塞主线程 → async_queue（52 ns 入队，后台消费）

序列化格式选择：
├─ 跨进程二进制通道 → to_binary（56 ns，cause 链完整，不含栈帧）
├─ 日志 / 人类可读 → to_string（144 ns）
└─ Web API / 跨语言 → to_json（198 ns baseline，带栈 3608 ns）
```

---

## 八、配置检查清单

部署前逐项确认：

- [ ] `plugin_registry_t::instance()` 在构造任何 `error_context_t` 前已调用
- [ ] 生产环境已关闭 `stacktrace` / `source_location` / `validation`
- [ ] 通知模式已按场景选择（普通业务 sync / 批处理 sync_deferred / 不阻塞 async_queue）
- [ ] 异步队列已设置 `set_max_queue_size` 防止内存爆炸
- [ ] `sync_deferred` 模式下请求处理结束已调用 `flush_deferred_notifications()`
- [ ] i18n 默认 locale 已设置（`set_default_locale`）
- [ ] 关键错误码已配置 `per_code_stacktrace_level` 差异化捕获
- [ ] 高频错误通道已接入 `error_dedup_sampler_t` 抑制洪水
- [ ] 单元测试 SetUp/TearDown 已重置所有配置项
- [ ] 跨进程传输路径已验证 binary 序列化 round-trip 一致性
