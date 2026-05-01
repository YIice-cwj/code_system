# 架构设计文档

本文档描述错误码系统的整体架构、模块职责划分及关键设计决策。

---

## 总体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        Error System                              │
├──────────────┬──────────────┬────────────────┬──────────────────┤
│  Core 层     │  i18n 层     │  Plugin 层     │  Utils 层        │
│  核心数据结构 │  多语言翻译  │  可扩展插件    │  通用工具        │
├──────────────┴──────────────┴────────────────┴──────────────────┤
│  Domain / Subsystem / Module / Traits 层                         │
│  预定义的 16 大系统域、子系统枚举、模块枚举及 Traits 转换        │
└─────────────────────────────────────────────────────────────────┘
```

### 依赖关系

```
Utils  ←─────────────────────────────── 被所有层使用
  ↑
Core (error_code_t, error_level_t)
  ↑                  ↑
Domain/Traits      error_context_t  ←─── Plugin 层监听
                        ↑
                     i18n 层（翻译 code → 文本）
```

> 核心原则：**下层不依赖上层**，`error_code_t` 不感知 i18n 和 Plugin。

---

## 层次说明

### Core 层 (`include/error_system/core/`)

| 文件 | 类/结构 | 职责 |
|------|---------|------|
| `error_code.h` | `error_code_t` | 64 位错误码的存储与字段解析 |
| `error_level.h` | `error_level_t` | 错误等级枚举及转换函数 |
| `error_builder.h` | `error_builder_t` | 编译期错误码构建工厂 |
| `error_context.h` | `error_context_t` | 错误上下文（码+消息+因果链） |
| `result_t.h` | `result_t<T>` | 类 Rust Result，替代异常传递错误 |
| `error_registry.h` | `error_registry_t` | 错误码注册（预留扩展） |

### Domain / Subsystem / Module / Traits 层

预定义 16 大系统域、每域的子系统枚举和模块枚举，并提供 Traits 类型萃取用于编译期枚举↔字符串↔整数的双向转换。

### i18n 层 (`include/error_system/i18n/`)

| 文件 | 类 | 职责 |
|------|----|------|
| `i_translator.h` | `i_translator_t` | 翻译器抽象接口 |
| `json_translator.h` | `json_translator_t` | 基于 JSON 字典的翻译实现 |
| `translator_registry.h` | `translator_registry_t` | 全局翻译器单例注册表 |
| `language.h` | `language_t` | 支持语言枚举 |

### Plugin 层 (`include/error_system/plugin/`)

| 文件 | 类 | 职责 |
|------|----|------|
| `i_error_plugin.h` | `i_error_plugin_t` | 插件抽象接口 |
| `plugin_registry.h` | `plugin_registry_t` | 插件单例注册表，负责广播 |

### Utils 层 (`include/error_system/utils/`)

| 文件 | 类/工具 | 职责 |
|------|---------|------|
| `string_utils.h` | `string_utils_t` | 哈希、格式化、分割、修剪等 |
| `json_utils.h` | `json_dict_t` | JSON 字典加载与点路径访问 |
| `file_utils.h` | 自由函数 | 文件读写 |

---

## 关键设计决策

### 1. 64 位 union + 位域实现零开销

```cpp
union error_code_union_t {
    uint64_t code;
    fields_t fields;  // 位域结构体
};
```

直接 `reinterpret` 整数与位域，字段读取等价于位运算，编译器可优化到单条指令。

### 2. `constexpr` 全链路

`error_builder_t::make_error_code()`、`to_string()`、`from_string()`、`hash()` 均为 `constexpr`，错误码可在编译期完全确定，进入只读数据段。

### 3. error_context_t 中的循环依赖打破

`plugin_registry.h` → `i_error_plugin.h` → `error_context.h`，若 `error_context.h` 直接 include `plugin_registry.h` 则循环。

**解法**：在 `error_context.h` 内仅声明自由函数 `__notify_plugins()`（无需完整定义 `plugin_registry_t`），其实现放在 `src/core/error_context.cc` 中，该 `.cc` 可安全 include `plugin_registry.h`。

```
error_context.h      ── 声明 __notify_plugins() ──→  头文件层无环
error_context.cc     ── include plugin_registry.h ──→ 仅在 .cc 连接
```

### 4. 全局单例注册表模式

i18n 的 `translator_registry_t` 和 Plugin 的 `plugin_registry_t` 均采用单例模式：
- **不持有所有权**：调用方负责对象生命周期
- **Meyer's singleton**：`static` 局部变量，C++11 保证线程安全的初始化

### 5. result_t\<T\> 无异常错误传递

```cpp
result_t<Data> fetch() {
    if (failed) return {error_code, "原因"};
    return data;
}

auto r = fetch();
if (r.is_error()) log(r.error().to_string());
```

避免异常开销，同时强制调用方显式处理错误。

---

## 扩展指南

### 新增系统域

1. 在 `domain/system_domain.h` 的 `system_domain_t` 枚举中添加值
2. 添加对应的子系统枚举文件（`subsystem/xxx_subsystem.h`）
3. 添加对应的模块枚举文件（`module/xxx_module.h`）
4. 添加 traits 文件（`traits/subsystem/xxx_subsystem_traits.h`、`traits/module/xxx_module_traits.h`）
5. 在 `json_translator.cc` 的 switch 中新增 case
6. 更新 JSON 字典文件

### 新增翻译语言

1. 在 `language_t` 枚举中新增语言值
2. 在 `languages/` 目录添加对应 JSON 字典文件（如 `ja_jp.json`）
3. 字典格式参考 `docs/api/i18n.md`

### 新增插件

1. 继承 `plugin::i_error_plugin_t`，实现 `name()` 和 `on_error()`
2. 在程序启动时调用 `plugin_registry_t::instance().register_plugin(&plugin)`
3. 参考 `docs/api/plugin.md` 中的示例
