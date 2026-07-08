# Plugin 层 API

`error_system::plugin`

插件层为错误事件提供可扩展的处理机制，支持同步、异步队列与延迟缓冲三种通知模式。通知模式的选择决策树详见 [决策树 · 1](../decision_tree.md#1-通知模式选择)。

---

## i_error_plugin_t

头文件：`error_system/plugin/i_error_plugin.h`

插件抽象接口，所有自定义插件必须继承。注册到 `plugin_registry_t` 后，由 `result_t::make_error` 经 `i_error_notifier_t::try_notify` 静态分发触发 `on_error()` 回调（`error_context_t` 构造函数本身不通知）；Lean 通知路径下仅传递 `error_code_t`，触发 `on_code()` 回调（默认空实现，按需 override）。通过 `min_level()` 在框架侧快速丢弃低等级事件，避免回调开销。

| 方法 | 签名 | 说明 |
|------|------|------|
| `name` | `[[nodiscard]] virtual std::string_view name() const noexcept = 0` | 插件唯一标识，同名注册替换旧插件 |
| `min_level` | `[[nodiscard]] virtual core::error_level_t min_level() const noexcept` | 低于此等级的事件被框架级过滤，默认 `debug`（接收所有） |
| `on_error` | `virtual void on_error(const core::error_context_t& context) noexcept = 0` | 错误事件回调（Full 路径），必须 `noexcept` |
| `on_code` | `virtual void on_code(core::error_code_t code) noexcept` | 错误码通知回调（Lean 路径），默认空实现。仅传递 `error_code_t`，不构造 `error_context_t`，零堆开销。插件可凭 code 从 `error_registry_t` 反查静态元数据 |

---

## plugin_registry_t

头文件：`error_system/plugin/plugin_registry.h`

插件单例注册表，管理所有已注册插件，实现 `core::i_error_notifier_t` 接口。热路径 RCU 快照零拷贝无锁读取（`shared_ptr<const vector>` + `atomic_load/store`）。

类型别名：`plugin_pointer_t`、`unique_plugin_ptr_t`、`shared_plugin_ptr_t`、`plugin_list_t`。

### 单例与状态

| 方法 | 签名 | 说明 |
|------|------|------|
| `instance` | `static plugin_registry_t& instance() noexcept` | 单例实例，首次调用自注册为默认通知器 |
| `is_initialized` | `[[nodiscard]] static bool is_initialized() noexcept` | 单例是否已初始化 |

### 注册与注销

| 方法 | 签名 | 说明 |
|------|------|------|
| `register_plugin` | `void register_plugin(unique_plugin_ptr_t plugin) noexcept` | 转移所有权，注册表管理生命周期 |
| `register_plugin_ref` | `void register_plugin_ref(i_error_plugin_t& plugin) noexcept` | 非持有引用，调用方负责生命周期 |
| `unregister_plugin` | `void unregister_plugin(std::string_view name) noexcept` | 按名称移除 |
| `clear` | `void clear() noexcept` | 清空所有已注册插件 |

### 通知分发（i_error_notifier_t 实现）

按 `feature_flags_t::get_notify_mode()` 分发到 sync / async_queue / sync_deferred 路径，调用方无需关心通知模式。

| 方法 | 签名 | 说明 |
|------|------|------|
| `notify`（上下文） | `void notify(const core::error_context_t& context) noexcept override` | Full 模式入口：sync 调 `notify_error`，async_queue 调 `enqueue_notification`，sync_deferred 调 `enqueue_deferred_notification` |
| `notify`（错误码） | `void notify(core::error_code_t code) noexcept override` | Lean 模式入口：全程不构造 `error_context_t`。sync 调 `notify_code_error_` 分发到 `on_code`，async_queue 调 `enqueue_code`，sync_deferred 调 `enqueue_deferred_code` |

### 同步通知

| 方法 | 签名 | 说明 |
|------|------|------|
| `notify_error` | `void notify_error(const core::error_context_t& context) noexcept` | RCU 快照无锁读取，依次调用 `on_error()`，回调异常被捕获 |

### 异步队列

| 方法 | 签名 | 说明 |
|------|------|------|
| `enqueue_notification` | `void enqueue_notification(const core::error_context_t& context) noexcept` | 推入后台队列，首次调用自动启动工作线程 |
| `pending_notifications` | `[[nodiscard]] size_t pending_notifications() const noexcept` | 待处理通知数量（context 通道 + code 通道之和，code 通道仅返回 0/1） |
| `set_max_queue_size` | `void set_max_queue_size(size_t max_size) noexcept` | 队列最大容量，0 = 无限制 |
| `get_max_queue_size` | `[[nodiscard]] size_t get_max_queue_size() const noexcept` | 队列最大容量 |

### 延迟缓冲（sync_deferred）

| 方法 | 签名 | 说明 |
|------|------|------|
| `enqueue_deferred_notification` | `void enqueue_deferred_notification(const core::error_context_t& context) noexcept` | 累积上下文到线程本地缓冲 |
| `enqueue_deferred_code` | `void enqueue_deferred_code(core::error_code_t code) noexcept` | 累积错误码到线程本地 code 缓冲（Lean 路径） |
| `flush_deferred_notifications` | `void flush_deferred_notifications() noexcept` | 触发当前线程累积通知（context + code 两路） |
| `pending_deferred_notifications` | `[[nodiscard]] size_t pending_deferred_notifications() const noexcept` | 待 flush 数量 |
| `clear_deferred_notifications` | `size_t clear_deferred_notifications() noexcept` | 清空缓冲（不触发通知），返回丢弃数 |
| `set_deferred_buffer_size` | `void set_deferred_buffer_size(size_t max_size) noexcept` | 缓冲最大容量，0 = 无限制（默认 1024） |
| `get_deferred_buffer_size` | `[[nodiscard]] size_t get_deferred_buffer_size() const noexcept` | 缓冲最大容量 |
| `deferred_buffer_overflowed` | `[[nodiscard]] bool deferred_buffer_overflowed() const noexcept` | 是否发生过溢出丢弃 |

### 状态查询

| 方法 | 签名 | 说明 |
|------|------|------|
| `size` | `[[nodiscard]] size_t size() const noexcept` | 已注册插件数量 |
| `empty` | `[[nodiscard]] bool empty() const noexcept` | 是否无插件 |

### 内部流程

`notify_error()` 复制回调指针后释放锁再调用 `on_error()`，避免持锁执行用户代码死锁；Lean 路径 `notify_code_error_()` 同理调用 `on_code()`。async_queue 队列满拒绝入队（背压）；sync_deferred 缓冲满丢弃并标记 overflow；`flushing` 标志防止 flush 期间递归入队。两条后台通道（context / code）各自管理独立工作线程，首次 `enqueue` 自动启动，析构自动 join；处理器异常隔离不退出。

```cpp
auto& registry = plugin::plugin_registry_t::instance();
registry.register_plugin(std::make_unique<my_plugin_t>());   // 转移所有权
my_plugin_t plugin;
registry.register_plugin_ref(plugin);                          // 非持有引用
registry.set_max_queue_size(10000);                            // 异步背压控制
registry.set_deferred_buffer_size(1024);                       // 延迟缓冲控制
registry.flush_deferred_notifications();                       // 显式 flush
```

`register_plugin(unique_ptr)` 接管所有权；`register_plugin_ref()` 不持有所有权。`sync_deferred` 模式下 `result_t::make_error` 会自动将通知入线程本地缓冲，无需手动调用 `enqueue_deferred_notification()`。

---

## async_notification_channel_t

头文件：`error_system/plugin/async_notification_channel.h`

异步通知通道，提供两条独立的后台通知路径：error_context 通道（基于 `async_queue_t`）与 error_code 通道（基于 `mpsc_queue_t` 无锁队列，专为 Lean 模式设计）。通过构造时注入回调打破与具体注册表的循环依赖。自动管理后台线程生命周期，支持背压控制。

类型别名：`context_ptr_t = std::shared_ptr<core::error_context_t>`，`notify_callback_t = std::function<void(const core::error_context_t&)>`，`code_callback_t = std::function<void(core::error_code_t)>`。

| 方法 | 签名 | 说明 |
|------|------|------|
| `async_notification_channel_t` | `explicit async_notification_channel_t(notify_callback_t callback, code_callback_t code_callback = {}) noexcept` | 注入 context 回调与 code 回调（code_callback 为空时不启动 code 通道） |
| `enqueue_notification` | `void enqueue_notification(const core::error_context_t& context) noexcept` | 异步入队上下文，首次自动启动工作线程 |
| `enqueue_code` | `void enqueue_code(core::error_code_t code) noexcept` | 异步入队纯错误码（Lean 路径），调用方数纳秒完成 |
| `pending_notifications` | `[[nodiscard]] size_t pending_notifications() const noexcept` | context 通道待处理数量 |
| `pending_codes` | `[[nodiscard]] size_t pending_codes() const noexcept` | code 通道待处理数量（仅 0/1，mpsc_queue 无 size 接口） |
| `set_max_queue_size` | `void set_max_queue_size(size_t max_size) noexcept` | context 队列最大容量，0 = 无限制 |
| `get_max_queue_size` | `[[nodiscard]] size_t get_max_queue_size() const noexcept` | context 队列最大容量 |

---

## error_router_plugin_t

头文件：`error_system/plugin/error_router_plugin.h`

按错误码 / 模块组 / 系统域将错误路由到不同处理函数，继承 `i_error_plugin_t`。双路径设计：Full 路径（`on_error`）路由到 `error_handler_t`，Lean 路径（`on_code`）路由到 `code_handler_t`，两路径 handler 独立注册、互不影响。线程安全（`std::shared_mutex`）。

类型别名（定义于 `error_system::plugin` 命名空间）：
- `error_handler_t = std::function<void(const core::error_context_t&)>`（Full 路径）
- `code_handler_t = std::function<void(core::error_code_t)>`（Lean 路径）

### 单例与接口

| 方法 | 签名 | 说明 |
|------|------|------|
| `instance` | `static error_router_plugin_t& instance() noexcept` | 路由插件单例 |
| `name` | `std::string_view name() const noexcept override` | 插件标识 |
| `on_error` | `void on_error(const core::error_context_t& context) noexcept override` | Full 路径回调，按 码 > 模块组 > 域 三级优先级匹配 `error_handler_t` |
| `on_code` | `void on_code(core::error_code_t code) noexcept override` | Lean 路径回调，按 码 > 模块组 > 域 三级优先级匹配 `code_handler_t`，未注册则静默跳过 |

### Full 路由注册（error_handler_t）

| 方法 | 签名 | 说明 |
|------|------|------|
| `register_handler_by_code` | `void register_handler_by_code(const core::error_code_t& code, error_handler_t handler) noexcept` | 按错误码注册 |
| `register_handler_by_module_group_id` | `void register_handler_by_module_group_id(core::module_group_id_t module_group_id, error_handler_t handler) noexcept` | 按模块组注册 |
| `register_handler_by_domain` | `void register_handler_by_domain(domain::system_domain_t domain, error_handler_t handler) noexcept` | 按系统域注册 |
| `unregister_handler_by_code` | `void unregister_handler_by_code(const core::error_code_t& code) noexcept` | 移除错误码路由 |
| `unregister_handler_by_module_group_id` | `void unregister_handler_by_module_group_id(core::module_group_id_t module_group_id) noexcept` | 移除模块组路由 |
| `unregister_handler_by_domain` | `void unregister_handler_by_domain(domain::system_domain_t domain) noexcept` | 移除系统域路由 |

### Lean 路由注册（code_handler_t）

| 方法 | 签名 | 说明 |
|------|------|------|
| `register_code_handler_by_code` | `void register_code_handler_by_code(const core::error_code_t& code, code_handler_t handler) noexcept` | 按错误码注册 Lean handler |
| `register_code_handler_by_module_group_id` | `void register_code_handler_by_module_group_id(core::module_group_id_t module_group_id, code_handler_t handler) noexcept` | 按模块组注册 Lean handler |
| `register_code_handler_by_domain` | `void register_code_handler_by_domain(domain::system_domain_t domain, code_handler_t handler) noexcept` | 按系统域注册 Lean handler |
| `unregister_code_handler_by_code` | `void unregister_code_handler_by_code(const core::error_code_t& code) noexcept` | 移除错误码 Lean 路由 |
| `unregister_code_handler_by_module_group_id` | `void unregister_code_handler_by_module_group_id(core::module_group_id_t module_group_id) noexcept` | 移除模块组 Lean 路由 |
| `unregister_code_handler_by_domain` | `void unregister_code_handler_by_domain(domain::system_domain_t domain) noexcept` | 移除系统域 Lean 路由 |

路由优先级（两路径一致）：错误码精确匹配 → 模块组匹配 → 系统域匹配。路由插件需通过 `register_plugin_ref()` 注册到 `plugin_registry_t` 后才会被通知。

---

## error_dedup_sampler_t

头文件：`error_system/plugin/error_dedup_sampler.h`

错误去重与采样器，对流入的错误上下文做去重与采样判定，不感知下游通知通道。线程安全，可被多生产者共享。去重基于 `error_code_t::get_identity_code()`（忽略 sign/reserved 位），采样采用确定性计数器（每 N 个放行 1 个），避免随机数开销。

类型别名：`clock_t = std::chrono::steady_clock`，`time_point_t = clock_t::time_point`。

### 配置

| 方法 | 签名 | 说明 |
|------|------|------|
| `set_dedup_window_ms` | `void set_dedup_window_ms(uint64_t milliseconds) noexcept` | 去重窗口(ms)，0 = 关闭（默认） |
| `set_sample_rate` | `void set_sample_rate(double rate) noexcept` | 采样率 [0.0, 1.0]，`rate=0.1` 放行 10%；极小 rate（1.0/rate 超出安全阈值）等同 0 全部抑制 |
| `should_be_forwarded` | `[[nodiscard]] bool should_be_forwarded(const core::error_context_t& context) noexcept` | 先采样再去重，两关通过才放行 |

### 统计

| 方法 | 签名 | 说明 |
|------|------|------|
| `deduped_count` | `[[nodiscard]] uint64_t deduped_count() const noexcept` | 被去重抑制数 |
| `sampled_count` | `[[nodiscard]] uint64_t sampled_count() const noexcept` | 被采样抑制数 |
| `forwarded_count` | `[[nodiscard]] uint64_t forwarded_count() const noexcept` | 放行数 |
| `reset_stats` | `void reset_stats() noexcept` | 重置统计与采样计数器（建议仅无并发/测试） |
| `clear_dedup_cache` | `void clear_dedup_cache() noexcept` | 清除去重表 |
| `dedup_cache_size` | `[[nodiscard]] size_t dedup_cache_size() const noexcept` | 去重表项数 |

`should_be_forwarded()` 非 `const`（内部更新计数与去重表）；`reset_stats()` 并发调用可能与 `should_be_forwarded()` 竞争，仅建议无并发/测试场景。

```cpp
plugin::error_dedup_sampler_t sampler;
sampler.set_dedup_window_ms(1000);   // 1s 内只放行一次
sampler.set_sample_rate(0.1);        // 放行 10%
if (sampler.should_be_forwarded(ctx)) { registry.enqueue_notification(ctx); }
```

---

## metric_plugin_t

头文件：`error_system/plugin/metric_plugin.h`

继承 `i_error_plugin_t`，按 code/level/subsystem 三维度统计错误次数。线程安全（`std::mutex`），`on_error()` / `on_code()` 共享同一计数路径（DRY），路径为 O(1) 原子递增 + 一次 map 查找。适用场景：错误率监控、热点错误码定位、子系统健康度评估。

### metric_snapshot_t

`snapshot()` 返回的不可变统计快照，拷贝后可独立访问。`level_counts` 索引与 `error_level_t` 数值对应：0=debug, 1=info, 2=warn, 3=error, 4=fatal。

| 字段 | 类型 | 说明 |
|------|------|------|
| `total_count` | `uint64_t` | 总错误数 |
| `level_counts` | `std::array<uint64_t, 5>` | 按等级计数（0=debug...4=fatal） |
| `code_counts` | `std::unordered_map<uint64_t, uint64_t>` | 按完整 64 位码计数 |
| `subsystem_counts` | `std::unordered_map<uint16_t, uint64_t>` | 按子系统计数 |

### 构造与特殊成员

| 成员 | 签名 | 说明 |
|------|------|------|
| 构造 | `explicit metric_plugin_t(string name = "metric", error_level_t min_level = error) noexcept` | 默认名称 `metric`，默认最低级别 `error` |
| 拷贝/移动 | 全部 `= delete` | 不可拷贝不可移动 |

### 方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `name` | `string_view name() const noexcept` | 插件名称 |
| `min_level` | `error_level_t min_level() const noexcept` | 最低关注级别 |
| `on_error` | `void on_error(const error_context_t&) noexcept` | 计数递增（委托 `count_code_`） |
| `on_code` | `void on_code(error_code_t code) noexcept` | Lean 路径计数递增，与 `on_error` 共享同一计数路径 |
| `snapshot` | `[[nodiscard]] metric_snapshot_t snapshot() const noexcept` | 导出统计快照 |
| `reset` | `void reset() noexcept` | 重置所有计数 |

```cpp
auto plugin = std::make_unique<plugin::metric_plugin_t>("app_metric", core::error_level_t::error);
plugin::plugin_registry_t::instance().register_plugin(std::move(plugin));
// ... 运行业务 ...
auto snap = metric_plugin.snapshot();
std::printf("total=%llu\n", static_cast<unsigned long long>(snap.total_count));
```

---

## log_plugin_t

头文件：`error_system/plugin/log_plugin.h`

继承 `i_error_plugin_t`，将错误上下文格式化输出到 `std::ostream`。支持 text / json 两种格式，`min_level()` 过滤低级别事件。线程安全（`std::mutex` 保护流写入，避免多线程交错输出）。不内置文件管理逻辑（KISS 原则），调用方负责流的生命周期；默认输出到 `std::cerr`。Lean 路径（`on_code`）仅输出 raw code，格式固定为 `[ERR: <code>]`，不查注册表、不分配字符串，保证热路径零开销。

输出格式枚举：`enum class format_t { text, json };`

### 构造与特殊成员

| 成员 | 签名 | 说明 |
|------|------|------|
| 构造 | `explicit log_plugin_t(string name = "logger", error_level_t min_level = warn, format_t format = text, ostream* stream = nullptr) noexcept` | 默认 `warn` + `text`，`stream=nullptr` 时使用 `std::cerr`（调用方需保证流生命周期长于插件） |
| 拷贝/移动 | 全部 `= delete` | 不可拷贝不可移动 |

### 方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `name` | `string_view name() const noexcept` | 插件名称 |
| `min_level` | `error_level_t min_level() const noexcept` | 最低输出级别 |
| `on_error` | `void on_error(const error_context_t&) noexcept` | 格式化并写入流 |
| `on_code` | `void on_code(error_code_t code) noexcept` | Lean 路径，输出 `[ERR: <code>]`，零字符串分配 |

```cpp
auto plugin = std::make_unique<plugin::log_plugin_t>("app_log",
                                                      core::error_level_t::warn,
                                                      plugin::log_plugin_t::format_t::json,
                                                      &file_stream);
plugin::plugin_registry_t::instance().register_plugin(std::move(plugin));
```

---

## 插件开发指南

### 三步速查

1. 继承 `i_error_plugin_t`，实现 `name()` 与 `on_error()`，可选重写 `on_code()`（Lean 路径）与 `min_level()` 做框架级过滤
2. 注册到 `plugin_registry_t`：`register_plugin(unique_ptr)` 转移所有权，或 `register_plugin_ref()` 引用注册
3. 保证线程安全：`on_error()` / `on_code()` 可能被多线程并发调用，使用 `std::atomic` 或锁保护内部状态

```cpp
class stats_plugin_t : public i_error_plugin_t {
    std::unordered_map<uint64_t, std::atomic<int>> counters_;
public:
    std::string_view name() const noexcept override { return "stats"; }
    core::error_level_t min_level() const noexcept override { return core::error_level_t::error; }
    void on_error(const core::error_context_t& ctx) noexcept override {
        ++counters_[ctx.get_code().get_code()];
    }
    // Lean 路径：仅需错误码的场景，避免构造完整 error_context_t
    void on_code(core::error_code_t code) noexcept override {
        ++counters_[code.get_code()];
    }
};
plugin::plugin_registry_t::instance().register_plugin(std::make_unique<stats_plugin_t>());
```

> Full 路径（`on_error`）由 `result_t<T, false>::make_error` 经 `try_notify` 触发；Lean 路径（`on_code`）由 `result_t<T, true>::make_error` 触发，仅传递 `error_code_t`。两路径互斥：同一次错误只会走其中一条，取决于 `result_t` 的 `Lean` 模板参数。`error_context_t` 构造函数本身不触发任何通知。
