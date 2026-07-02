# Config 层 API

`error_system::config`

原 `error_config_t` 已按单一职责原则拆分为四个独立配置类，`error_config.h` 仅作向后兼容的统一包含入口。新代码请直接包含对应的细分头文件。

---

## feature_flags_t

特性开关配置类，管理编译期特性开关与运行时布尔标志位。编译期开关通过 `if constexpr` 消除运行时开销，编译器死代码消除未启用分支。类仅含静态成员，禁止实例化。

文本/i18n 输出开关已迁移到 `i18n_config_t::set_enable_i18n`，`feature_flags_t` 不再保留 `set_enable_text_output`。

### notify_mode_t

| 枚举值 | 说明 |
|--------|------|
| `sync` | 同步通知（默认） |
| `async_queue` | 异步队列通知（后台线程消费） |
| `sync_deferred` | 同步延迟通知（线程本地缓冲 + 显式 flush） |

三种通知模式的选择决策树详见 [决策树 · 1](../decision_tree.md#1-通知模式选择)。

### API

| 成员 | 签名 | 说明 |
|------|------|------|
| STACKTRACE_ENABLED | `static constexpr bool` | 编译期堆栈追踪开关 |
| VALIDATION_ENABLED | `static constexpr bool` | 编译期错误码验证开关 |
| LOCATION_ENABLED | `static constexpr bool` | 编译期源位置追踪开关 |
| set_enable_stacktrace | `static void set_enable_stacktrace(bool enable) noexcept` | 开启/关闭堆栈追踪 |
| is_stacktrace_enabled | `[[nodiscard]] static bool is_stacktrace_enabled() noexcept` | 堆栈追踪是否开启 |
| set_enable_validation | `static void set_enable_validation(bool enable) noexcept` | 开启/关闭错误码验证 |
| is_validation_enabled | `[[nodiscard]] static bool is_validation_enabled() noexcept` | 验证是否开启 |
| set_enable_source_location | `static void set_enable_source_location(bool enable) noexcept` | 开启/关闭源位置追踪 |
| is_source_location_enabled | `[[nodiscard]] static bool is_source_location_enabled() noexcept` | 源位置追踪是否开启 |
| set_enable_short_filename | `static void set_enable_short_filename(bool enable) noexcept` | 开启/关闭短文件名模式 |
| is_short_filename_enabled | `[[nodiscard]] static bool is_short_filename_enabled() noexcept` | 短文件名模式是否开启 |
| set_notify_mode | `static void set_notify_mode(notify_mode_t mode) noexcept` | 设置通知模式 |
| get_notify_mode | `[[nodiscard]] static notify_mode_t get_notify_mode() noexcept` | 获取通知模式 |

若编译期未启用对应特性，相关 `set_*` 调用无操作，`is_*_enabled()` 始终返回 `false`。

---

## stacktrace_config_t

堆栈追踪配置类，管理全局堆栈阈值与 per-code 覆盖配置。类仅含静态成员，禁止实例化。

### API

| 方法 | 签名 | 说明 |
|------|------|------|
| get_stacktrace_level | `[[nodiscard]] static core::error_level_t get_stacktrace_level() noexcept` | 获取全局堆栈阈值 |
| set_stacktrace_level | `static void set_stacktrace_level(core::error_level_t level) noexcept` | 设置全局堆栈阈值 |
| set_per_code_stacktrace_level | `static void set_per_code_stacktrace_level(uint64_t identity_code, core::error_level_t level) noexcept` | 设置 per-code 覆盖 |
| get_per_code_stacktrace_level | `[[nodiscard]] static std::optional<core::error_level_t> get_per_code_stacktrace_level(uint64_t identity_code) noexcept` | 查询 per-code 覆盖 |
| remove_per_code_stacktrace_level | `static void remove_per_code_stacktrace_level(uint64_t identity_code) noexcept` | 删除 per-code 覆盖 |

### per-code 优先级

```
is_stacktrace_enabled() == false  →  不捕获
否则 → per-code 有值 ? per-code : 全局阈值
错误等级 >= 最终阈值 → 捕获堆栈
```

per-code 覆盖不影响全局 `is_stacktrace_enabled()` 判断；编译期未启用堆栈追踪时，`set_*` 无操作，`get_*` 返回 `warn` / `std::nullopt`。

---

## formatter_config_t

自定义格式化器配置类，管理错误上下文的自定义格式化函数。自定义格式化器在 `error_context_serializer_t::to_string()` 内部调用，返回非空则替换默认文本输出。

### 类型别名

| 类型 | 签名 |
|------|------|
| formatter_callback_t | `using formatter_callback_t = std::function<std::string(const core::error_context_t&)>;` |

### API

| 方法 | 签名 | 说明 |
|------|------|------|
| set_custom_formatter | `static void set_custom_formatter(formatter_callback_t formatter) noexcept` | 设置格式化函数（nullptr 清除，恢复默认） |
| get_custom_formatter | `[[nodiscard]] static formatter_callback_t get_custom_formatter() noexcept` | 获取格式化函数副本（线程安全） |

---

## i18n_config_t

i18n 配置类，管理 i18n 启用开关与输出 locale 配置。类仅含静态成员，禁止实例化，使用 `std::atomic` 无锁读写。本类仅持有配置状态，不直接调用 `i18n_t` / `error_registry_t`，序列化器从本类读取 locale 后显式传给 registry / i18n_t 查询本地化文本。

### API

| 方法 | 签名 | 说明 |
|------|------|------|
| set_enable_i18n | `static void set_enable_i18n(bool enable) noexcept` | i18n 总开关（false 时序列化输出回退为原始 ID 数字） |
| is_i18n_enabled | `[[nodiscard]] static bool is_i18n_enabled() noexcept` | i18n 是否启用 |
| set_default_locale | `static void set_default_locale(locale_t locale) noexcept` | 设置默认 locale（回退查询使用） |
| get_default_locale | `[[nodiscard]] static locale_t get_default_locale() noexcept` | 获取默认 locale |
| set_output_locale | `static void set_output_locale(locale_t locale) noexcept` | 设置输出 locale（运行时切换语言） |
| clear_output_locale | `static void clear_output_locale() noexcept` | 清除输出 locale，回退到默认 |
| get_output_locale | `[[nodiscard]] static std::optional<locale_t> get_output_locale() noexcept` | 获取显式设置的输出 locale |
| resolve_output_locale | `[[nodiscard]] static locale_t resolve_output_locale() noexcept` | 解析最终输出 locale |

### locale 解析顺序

```
resolve_output_locale()
1. output_locale 已设置 → 返回 output_locale
2. 否则 → 返回 default_locale
```

```cpp
i18n_config_t::set_enable_i18n(true);
i18n_config_t::set_default_locale(locale_t::zh_CN);
i18n_config_t::set_output_locale(locale_t::en_US);
i18n_config_t::resolve_output_locale();   // → en_US
i18n_config_t::clear_output_locale();
i18n_config_t::resolve_output_locale();   // → zh_CN
```

---

## 默认配置

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| `min_stacktrace_level_` | `error` | 自动捕获堆栈的最低等级 |
| `enable_stacktrace_` | `true` | 堆栈追踪总开关 |
| `enable_validation_` | `true` | 错误码验证 |
| `enable_source_location_` | `true` | 源位置追踪 |
| `enable_short_filename_` | `true` | 短文件名模式 |
| `notify_mode_` | `sync` | 插件通知模式 |

---

## 配置场景速查

**开发环境**

```cpp
using namespace error_system::config;
stacktrace_config_t::set_stacktrace_level(core::error_level_t::debug);
feature_flags_t::set_enable_stacktrace(true);
```

**生产环境**

```cpp
stacktrace_config_t::set_stacktrace_level(core::error_level_t::error);
feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::async_queue);
i18n_config_t::set_enable_i18n(true);
```

**请求处理批处理（sync_deferred）**

```cpp
feature_flags_t::set_notify_mode(feature_flags_t::notify_mode_t::sync_deferred);
// 请求处理（构造 error_context_t 自动入线程本地缓冲）
plugin::plugin_registry_t::instance().flush_deferred_notifications();
```

**高性能场景**

```cpp
feature_flags_t::set_enable_stacktrace(false);
feature_flags_t::set_enable_source_location(false);
feature_flags_t::set_enable_validation(false);
i18n_config_t::set_enable_i18n(false);  // 数字 ID 输出
```

**差异化堆栈**

```cpp
stacktrace_config_t::set_stacktrace_level(core::error_level_t::error);
stacktrace_config_t::set_per_code_stacktrace_level(
    ERR_CRITICAL.get_identity_code(), core::error_level_t::debug);
```

**自定义格式化器**

```cpp
formatter_config_t::set_custom_formatter(
    [](const core::error_context_t& e) { return e.message; });
formatter_config_t::set_custom_formatter(nullptr);  // 恢复默认
```

---

## CMake 编译选项

编译期宏（`ERROR_SYSTEM_ENABLE_STACKTRACE` / `VALIDATION` / `LOCATION`）的完整选项与构建说明详见 [架构设计 · 编译配置](../architecture.md#编译配置)。

编译期宏通过 `feature_flags_t::STACKTRACE_ENABLED` / `VALIDATION_ENABLED` / `LOCATION_ENABLED`（`public constexpr bool`）暴露，`error_context_t` 内部使用 `if constexpr` 替代 `#ifdef`，编译器死代码消除未启用分支。
