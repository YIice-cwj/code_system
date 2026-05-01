# Core 层 API 文档

> 命名空间：`error_system::core`

Core 层是整个错误系统的基础，定义了错误码数据结构、错误等级、错误上下文及构建器。

---

## error_code_t

**头文件**：`error_system/core/error_code.h`

64 位错误码的核心数据类，使用 `union` + 位域实现零开销的字段拆解。

### 位域结构

| 位区间 | 长度 | 字段 | 描述 |
|--------|------|------|------|
| `63` | 1 bit | `sign` | 符号位（1 = 错误） |
| `60–62` | 3 bit | `reserved` | 预留位 |
| `56–59` | 4 bit | `level` | 错误等级 |
| `48–55` | 8 bit | `system` | 系统域 |
| `32–47` | 16 bit | `subsys` | 子系统编号 |
| `16–31` | 16 bit | `module` | 模块编号 |
| `0–15` | 16 bit | `number` | 具体错误编号 |

### 方法

| 方法 | 返回值 | 描述 |
|------|--------|------|
| `get_code()` | `uint64_t` | 获取原始 64 位码 |
| `get_sign()` | `uint8_t` | 获取符号位 |
| `get_level()` | `error_level_t` | 获取错误等级 |
| `get_system()` | `system_domain_t` | 获取系统域 |
| `get_subsys()` | `uint16_t` | 获取子系统编号 |
| `get_module()` | `uint16_t` | 获取模块编号 |
| `get_number()` | `uint16_t` | 获取错误编号 |

### 示例

```cpp
error_code_t code(raw_value);
if (code.get_sign() == 1) {
    auto level = code.get_level();   // error_level_t::fatal
    auto sys   = code.get_system();  // system_domain_t::database
    auto num   = code.get_number();  // 404
}
```

---

## error_level_t

**头文件**：`error_system/core/error_level.h`

错误等级枚举，对应错误码位域中的 `level` 字段。

| 枚举值 | 整数值 | 描述 |
|--------|--------|------|
| `debug` | 0 | 调试信息 |
| `info` | 1 | 一般信息 |
| `warn` | 2 | 警告 |
| `error` | 3 | 错误 |
| `fatal` | 4 | 致命错误 |
| `custom` | 5 | 自定义 |

### 辅助函数

```cpp
// 枚举 ↔ 字符串
constexpr const char* to_string(error_level_t level);
constexpr error_level_t from_string(const char* str);

// 枚举 ↔ 整数
constexpr uint8_t to_int(error_level_t level);
constexpr error_level_t from_int(uint8_t value);

// 等级过滤
constexpr bool should_log(error_level_t current, error_level_t min_level);
```

---

## error_builder_t

**头文件**：`error_system/core/error_builder.h`

构建 `error_code_t` 的工厂类，所有方法均为 `constexpr`，支持编译期构建。

### 方法

```cpp
// 按字段构建
static constexpr error_code_t make_error_code(
    error_level_t level,
    domain::system_domain_t system,
    uint16_t subsys,
    uint16_t module,
    uint16_t number
) noexcept;

// 从原始 64 位码构建
static constexpr error_code_t make_error_code(uint64_t code) noexcept;
```

### 示例

```cpp
// 编译期常量，零运行时开销
constexpr auto db_conn_err = error_builder_t::make_error_code(
    error_level_t::fatal,
    domain::system_domain_t::database,
    100,   // subsystem id
    200,   // module id
    404    // error number
);
```

---

## error_context_t

**头文件**：`error_system/core/error_context.h`

错误上下文，封装错误码、消息文本及因果链（cause chain）。

### 成员

| 成员 | 类型 | 描述 |
|------|------|------|
| `code` | `error_code_t` | 错误码 |
| `message` | `std::string` | 错误描述文本 |
| `cause` | `shared_ptr<error_context_t>` | 上级原因（可选） |

### 方法

```cpp
// 判断是否为有效错误（code != 0）
explicit operator bool() const noexcept;

// 包装因果链：将 underlying 作为 this 的 cause
error_context_t wrap(const error_context_t& underlying) const;

// 转为可读字符串
// translator 优先级：传入参数 > 全局注册翻译器 > 降级英文名
std::string to_string(const i18n::i_translator_t* translator = nullptr) const;
```

### 插件触发

构造 `error_context_t` 时（`code != 0`），会自动调用 `plugin::plugin_registry_t::instance().notify_error()`，通知所有已注册插件。

### 示例

```cpp
error_context_t ctx{db_err, "数据库连接超时"};

// 包装因果链
error_context_t outer{app_err, "请求处理失败"};
auto chained = outer.wrap(ctx);

// 转字符串（使用全局翻译器）
std::cout << chained.to_string() << std::endl;
// [Level: fatal, System: database, ...] Code: 404 - 数据库连接超时
//   ↳ Caused by: ...
```

---

## result_t\<T\>

**头文件**：`error_system/core/result_t.h`

类 Rust `Result` 的返回值封装，持有成功值或错误上下文，不使用异常。

```cpp
// 泛型版本
result_t<int> r1 = 42;                          // 成功
result_t<int> r2 = {some_code, "失败原因"};     // 错误

r1.is_success(); // true
r2.is_error();   // true
r2.error();      // error_context_t&
r1.value();      // int&

// void 特化（无成功值）
result_t<void> r3;                              // 成功（code == 0）
result_t<void> r4 = {some_code, "操作失败"};   // 错误
```
