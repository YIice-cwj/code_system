# Core 层 API

命名空间 `error_system::core`。本层提供错误码、错误上下文、结果类型与注册表的纯数据抽象，不依赖任何外部 IO 或日志后端。所有方法 `noexcept`，`std::bad_alloc` 等异常在内部捕获并记录到 `stderr`，对象保持可安全析构状态。

> **v4.0.0 头文件布局**：`core/error_code.h` · `core/error_context.h` · `core/runtime_block.h` · `core/error_context_initializer.h` · `core/error_exception.h` · `core/error_level.h` · `core/error_metadata.h` · `core/error_builder.h` · `core/i_error_notifier.h` · `core/result/result.h` · `core/registry/error_registry.h` · `core/registry/duplicate_policy.h` · `core/serializer/error_context_serializer.h`

---

## error_code_t

64 位错误码数据类。基于位移与掩码实现字段解析，100% 避免严格别名与位域 UB。默认构造为成功码（sign=0，其余字段为 0，遵循 Unix 约定：0=成功，非0=失败），可作为函数成功返回值的零成本默认。

### 位域布局

| 位域 | 位数 | 说明 |
|------|:---:|------|
| Sign | 1 | `0` = 成功 · `1` = 失败 |
| Reserved | 3 | bit0 `retryable` · bit1 `transient` · bit2 预留 |
| Level | 4 | debug · info · warn · error · fatal |
| System Domain | 8 | 6 大系统域 |
| Subsystem | 16 | 子系统 ID |
| Module | 16 | 模块 ID |
| Number | 16 | 错误编号 |

### 类型别名与强类型包装

| 类型 | 定义 | 说明 |
|------|------|------|
| `code_t` | `using code_t = uint64_t;` | 64 位原始错误码 |
| `module_group_id_t` | `using module_group_id_t = uint64_t;` | 模块组聚合 ID |
| `subsystem_id_t` | `struct { uint16_t value{0}; explicit constexpr subsystem_id_t(uint16_t) noexcept; };` | 子系统 ID 强类型包装 |
| `module_id_t` | `struct { uint16_t value{0}; explicit constexpr module_id_t(uint16_t) noexcept; };` | 模块 ID 强类型包装 |
| `error_number_t` | `struct { uint16_t value{0}; explicit constexpr error_number_t(uint16_t) noexcept; };` | 错误编号强类型包装 |

`subsystem_id_t` / `module_id_t` / `error_number_t` 的 `explicit` 构造防止三个 `uint16_t` 参数传反。

### 构造方法

| 方法 | 签名 | 说明 |
|------|------|------|
| 默认构造 | `constexpr error_code_t() noexcept` | sign=0 的成功码 |
| 原始码构造 | `constexpr explicit error_code_t(code_t code) noexcept` | 直接传入 64 位原始值 |
| 五参构造 | `constexpr error_code_t(error_level_t level, domain::system_domain_t system, subsystem_id_t subsystem, module_id_t module, error_number_t number) noexcept` | 按字段构造，sign=1（失败） |
| 成功码工厂 | `static constexpr error_code_t make_success() noexcept` | 等价默认构造，语义更清晰 |
| 拷贝/移动构造 | `= default`（全部 `noexcept`） | Rule of 5 |
| 拷贝/移动赋值 | `= default`（全部 `noexcept`） | Rule of 5 |

### 字段访问

| 方法 | 签名 | 说明 |
|------|------|------|
| `get_code` | `[[nodiscard]] constexpr code_t get_code() const noexcept` | 原始 64 位码 |
| `get_identity_code` | `[[nodiscard]] constexpr code_t get_identity_code() const noexcept` | 清除 sign/reserved 的业务标识码 |
| `get_level` | `[[nodiscard]] constexpr error_level_t get_level() const noexcept` | 错误等级（非法值回退 fatal） |
| `get_system` | `[[nodiscard]] constexpr domain::system_domain_t get_system() const noexcept` | 系统域（非法值回退 none） |
| `get_subsys` | `[[nodiscard]] constexpr uint16_t get_subsys() const noexcept` | 子系统 ID |
| `get_module` | `[[nodiscard]] constexpr uint16_t get_module() const noexcept` | 模块 ID |
| `get_number` | `[[nodiscard]] constexpr uint16_t get_number() const noexcept` | 错误编号 |
| `get_module_group_id` | `[[nodiscard]] constexpr module_group_id_t get_module_group_id() const noexcept` | 模块组聚合 ID（系统+子系统+模块） |

### 谓词与转换

| 方法 | 签名 | 说明 |
|------|------|------|
| `is_success_code` | `[[nodiscard]] constexpr bool is_success_code() const noexcept` | sign==0 |
| `is_error_code` | `[[nodiscard]] constexpr bool is_error_code() const noexcept` | sign!=0 |
| `is_retryable` | `[[nodiscard]] constexpr bool is_retryable() const noexcept` | Reserved.bit0 |
| `is_transient` | `[[nodiscard]] constexpr bool is_transient() const noexcept` | Reserved.bit1 |
| `operator code_t` | `explicit constexpr operator code_t() const noexcept` | 显式转原始 64 位码 |
| `operator==` / `!=` / `<` | `[[nodiscard]] constexpr bool operator==(const error_code_t&) const noexcept` | 按原始码比较 |

### 位操作

| 方法 | 签名 | 说明 |
|------|------|------|
| `get_sign` | `[[nodiscard]] constexpr uint8_t get_sign() const noexcept` | 符号位（0=成功，1=失败） |
| `get_reserved` | `[[nodiscard]] constexpr uint8_t get_reserved() const noexcept` | 预留位（0-7） |
| `set_sign` | `constexpr void set_sign(uint8_t sign) noexcept` | 设置符号位（超范围视为 1 即失败） |
| `set_reserved` | `constexpr void set_reserved(uint8_t reserved) noexcept` | 设置预留位（超范围视为 0） |
| `set_retryable` | `constexpr void set_retryable(bool retryable) noexcept` | 写入 Reserved.bit0 |
| `set_transient` | `constexpr void set_transient(bool transient) noexcept` | 写入 Reserved.bit1 |

### 示例

```cpp
error_code_t code(error_level_t::error, system_domain_t::database,
                  subsystem_id_t{1}, module_id_t{2}, error_number_t{0x0010});
code.set_retryable(true);
if (code.is_retryable()) { /* 重试逻辑 */ }
auto ok = error_code_t::make_success();
```

### 编译期冲突检测

```cpp
template <size_t N>
[[nodiscard]] constexpr bool all_unique(const std::array<error_code_t, N>& codes) noexcept;
```

O(n²) constexpr 暴力比较，配合 `static_assert` 在编译期捕获重复错误码定义：

```cpp
constexpr std::array<error_code_t, 3> codes = {ERR_A, ERR_B, ERR_C};
static_assert(all_unique(codes), "Duplicate error codes detected");
```

---

## error_context_t

错误上下文数据类（24 字节 Move-Only）。封装错误码 + 动态运行时块 + 因果链。

- **物理布局严格 24 字节**：8B `error_code` + 8B `runtime_block_t*` + 8B `cause*`
- **动静分离**：静态数据（`error_code`）内联，动态数据（message/payload/stack/source_location/metadata）收拢到 `runtime_block_t` 堆块，按需分配（`block_ == nullptr` 表示无动态数据，零开销）
- **Move-Only**：禁用拷贝构造/赋值，仅支持移动；如需共享，调用方自行用 `shared_ptr` 包装或调用 `clone()`
- **因果链**：采用 `std::unique_ptr<error_context_t>` 独占所有权，零引用计数
- 序列化职责委托给 `error_context_serializer_t`，运行时特性初始化委托给 `error_context_initializer_t`，遵循单一职责原则
- `source_location` / `file_name` / `stack_frames` 的填充由编译期特性开关（`LOCATION_ENABLED` / `STACKTRACE_ENABLED`）控制，内部使用 `if constexpr` 由编译器死代码消除未启用分支

### located_code_t

携带源位置的 `error_code_t` 包装。构造函数未标记 `explicit`，允许 `error_code_t` 隐式转换为 `located_code_t`，使 `error_context_t` 可直接接受 `error_code_t` 作为首参数。从 `error_code_t` 隐式构造时通过 `source_location_t::current()` 自动捕获调用者位置。

| 成员 | 类型 | 说明 |
|------|------|------|
| `code` | `error_code_t` | 错误码 |
| `location` | `utils::source_location_t` | 源位置（默认 `current()`） |

构造签名：`located_code_t(error_code_t code, utils::source_location_t location = utils::source_location_t::current()) noexcept`

### runtime_block_t

动态运行时上下文堆块（公开类型，头文件 `core/runtime_block.h`）。持有 `error_context_t` 的所有动态字段，字段为 public，仅供 `error_context_t` 及其友元（serializer/initializer）直接访问。按需分配：构造成功码或 `make_minimal` 时不分配（`block_ == nullptr`），仅在需要消息/payload/堆栈等动态数据时分配。

| 成员 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `message` | `std::string` | `{}` | 错误消息 |
| `metadata` | `std::optional<error_metadata_t>` | `nullopt` | 错误码元数据（懒查询缓存） |
| `source_location` | `utils::source_location_t` | `{}` | 源位置（由 `LOCATION_ENABLED` 控制） |
| `file_name` | `const char*` | `nullptr` | 文件名指针，指向 `loc_file_storage` 或字面量（由 `LOCATION_ENABLED` 控制） |
| `loc_file_storage` | `std::string` | `{}` | 反序列化场景下的文件名存储（保证 `file_name` 生命周期安全） |
| `loc_func_storage` | `std::string` | `{}` | 反序列化场景下的函数名存储 |
| `raw_frames` | `std::shared_ptr<const std::vector<void*>>` | `nullptr` | 原始栈帧指针（capture 时存入） |
| `resolved_frames` | `std::shared_ptr<const std::vector<std::string>>` | `nullptr` | 已符号化栈帧字符串（测试接口或延迟符号化结果） |
| `payload_count` | `uint8_t` | `0` | payload 项数 |
| `payload_small` | `std::array<std::pair<std::string, std::string>, 4>` | `{}` | SSO 区，前 4 项内联 |
| `payload_overflow` | `std::unique_ptr<std::unordered_map<std::string, std::string>>` | `nullptr` | 溢出区 |

栈帧延迟符号化：`raw_frames` 与 `resolved_frames` 互斥——生产路径用 `raw_frames`，输出时按需 resolve（带 `thread_local` 缓存）；测试路径用 `resolved_frames`。均使用 `shared_ptr<const>` 实现零拷贝共享。

深拷贝：`[[nodiscard]] static std::unique_ptr<runtime_block_t> deep_copy(const runtime_block_t&)`，用于 `error_context_t::clone()` 和 `wrap()` 场景。

静态常量 `error_context_t::PAYLOAD_SSO_CAPACITY = 4`：负载项 ≤ 4 时栈上存储零堆分配，超出后溢出到 `payload_overflow`。**v4.0.0 起该容量为 `runtime_block_t` 类型布局的硬编码常量**（`std::array` 大小是类型的一部分），不再支持通过 `-DERROR_SYSTEM_PAYLOAD_SSO_CAPACITY=N` 宏覆盖；如需调整需修改 `runtime_block.h` 并重新编译整个库。

### 构造

| 方法 | 签名 | 说明 |
|------|------|------|
| 默认构造 | `error_context_t() noexcept = default` | 成功码空上下文，`block_ == nullptr` |
| located_code 构造 | `template <typename... Args> error_context_t(located_code_t lc, std::string message_format, Args&&... args) noexcept` | 自动捕获源位置、格式化消息、触发初始化；成功码跳过初始化且不分配 `block_` |
| 异常转换工厂 | `[[nodiscard]] static error_context_t from_exception(error_code_t code, const std::exception& e, utils::source_location_t loc = current()) noexcept` | 从 `std::exception` 创建 |
| `make_minimal` | `[[nodiscard]] static error_context_t make_minimal(error_code_t code, utils::source_location_t loc = current()) noexcept` | 跳过 validation/stacktrace/notification，仅分配 `block_` 存 `source_location`，仅供 Lean 模式读取路径使用 |
| `clone` | `[[nodiscard]] error_context_t clone() const noexcept` | 深拷贝（含 cause 链递归克隆）。Move-Only 语义下不可拷贝，需显式调用；用于通知系统异步入队、延迟缓冲等场景。分配失败时返回部分拷贝 |
| 拷贝构造 | `error_context_t(const error_context_t&) = delete` | 禁用（Move-Only） |
| 移动构造 | `error_context_t(error_context_t&&) noexcept = default` | 移动所有权，源对象置空 |
| 拷贝赋值 | `error_context_t& operator=(const error_context_t&) = delete` | 禁用（Move-Only） |
| 移动赋值 | `error_context_t& operator=(error_context_t&&) noexcept = default` | 自赋值安全 |

### 动态块与因果链访问

| 方法 | 签名 | 说明 |
|------|------|------|
| `block` | `[[nodiscard]] const runtime_block_t* block() const noexcept` | 只读访问动态块，未分配时返回 nullptr |
| `cause` | `[[nodiscard]] const error_context_t* cause() const noexcept` | 只读访问因果链下游节点，无因果链时返回 nullptr |
| `get_message` | `[[nodiscard]] const std::string& get_message() const noexcept` | 消息引用，未分配 `block_` 时返回空字符串哨兵 |
| `get_file_name` | `[[nodiscard]] const char* get_file_name() const noexcept` | 文件名指针，未分配 `block_` 时返回 nullptr |
| `get_source_location` | `[[nodiscard]] const utils::source_location_t& get_source_location() const noexcept` | 源位置引用，未分配 `block_` 时返回空哨兵 |
| `get_stack_frames` | `[[nodiscard]] std::shared_ptr<const std::vector<std::string>> get_stack_frames() const noexcept` | 堆栈帧（延迟符号化，未分配或无帧时返回 nullptr） |
| `with_stack_frames` | `error_context_t& with_stack_frames(std::vector<std::string> frames) noexcept` | 测试场景手动设置栈帧，链式 |

### 负载操作

`with()` 共 7 个重载，全部返回 `error_context_t&` 支持链式调用，合并为下表两行：

| 方法 | 签名（合并重载） | 说明 |
|------|------|------|
| `with`（字符串） | `error_context_t& with(string/string_view/const char* key, string/string_view/const char* value) noexcept` | 添加字符串负载（4 个重载） |
| `with`（模板） | `template <typename T> error_context_t& with(string/string_view/const char* key, T value) noexcept` | 添加任意类型负载（bool/int/double 等，3 个重载） |
| `with_batch` | `error_context_t& with_batch(std::initializer_list<std::pair<const std::string, std::string>> items) noexcept` | 批量添加 |
| `for_each_payload` | `template <typename Visitor> void for_each_payload(Visitor&& visitor) const noexcept` | 遍历所有 payload 项 |
| `get_payload` | `[[nodiscard]] std::vector<std::pair<std::string, std::string>> get_payload() const noexcept` | 获取 payload 副本 |
| `get_payload_value` | `[[nodiscard]] std::optional<std::string> get_payload_value(const std::string& key) const noexcept` | 按 key 取值 |
| `payload_size` | `[[nodiscard]] size_t payload_size() const noexcept` | 项数 |
| `is_payload_empty` | `[[nodiscard]] bool is_payload_empty() const noexcept` | 是否为空 |

### 查询与谓词

| 方法 | 签名 | 说明 |
|------|------|------|
| `get_code` | `[[nodiscard]] const error_code_t& get_code() const noexcept` | 错误码只读引用 |
| `is_success` | `bool is_success() const noexcept` | sign==0 |
| `is_error` | `bool is_error() const noexcept` | sign!=0 |
| `is_fatal` | `[[nodiscard]] bool is_fatal() const noexcept` | level == fatal |
| `is_retryable` | `[[nodiscard]] bool is_retryable() const noexcept` | 委托 error_code_t |
| `is_transient` | `[[nodiscard]] bool is_transient() const noexcept` | 委托 error_code_t |
| `what` | `[[nodiscard]] const char* what() const noexcept` | message 的 c_str |

### 谓词比较

| 方法 | 签名 | 说明 |
|------|------|------|
| `equals_by_code` | `[[nodiscard]] bool equals_by_code(const error_context_t&) const noexcept` | O(1) 仅按错误码 |
| `equals_strict` | `[[nodiscard]] bool equals_strict(const error_context_t&) const noexcept` | 含 cause 链 + stack_frames 深比较 |
| `operator==` | `[[nodiscard]] bool operator==(const error_context_t&) const noexcept` | 按 code/message/payload 比较 |
| `operator!=` | `[[nodiscard]] bool operator!=(const error_context_t&) const noexcept` | 取反 |

### 因果链与聚合

| 方法 | 签名 | 说明 |
|------|------|------|
| `wrap` | `[[nodiscard]] error_context_t wrap(error_context_t&& underlying) const noexcept` | 返回包含 cause 的新对象：当前对象深拷贝，`underlying` 移动到新对象的 cause 链（含自环检测，避免循环引用） |
| `join_errors` | `[[nodiscard]] error_context_t join_errors(std::vector<error_context_t>&& errors) noexcept` | 批量校验聚合，主错误取首个，其余以 `joined_error_N` 为键附加 |

### 序列化便捷方法

委托 `error_context_serializer_t`，免 include serializer 头文件。

| 方法 | 签名 | 说明 |
|------|------|------|
| `to_string` | `[[nodiscard]] std::string to_string() const noexcept` | 可读文本 |
| `to_json` | `[[nodiscard]] std::string to_json() const noexcept` | JSON |
| `to_binary` | `[[nodiscard]] std::string to_binary() const noexcept` | 紧凑二进制 |

### 示例

```cpp
error_context_t ctx(ERR_DB_TIMEOUT, "连接超时: {}ms", 3000);
ctx.with("host", "192.168.1.1").with("port", 3306);
auto chained = biz_error.wrap(std::move(db_error));  // wrap 接受右值
auto host = ctx.get_payload_value("host");  // std::optional
```

```cpp
std::vector<error_context_t> errs;
if (!validate_a()) errs.push_back(std::move(err_a));  // Move-Only：必须移动
if (!validate_b()) errs.push_back(std::move(err_b));
return join_errors(std::move(errs));
```

---

## result_t<T, bool Lean = false>

类 Rust Result，零异常错误传递。**v4.0.0 起内部使用 `union` + `result_state_t` 手写判别式替代 `std::variant`**——因 `error_context_t` 现为 Move-Only，而 C++17 `std::variant` 要求元素可拷贝。`error_storage_t = std::conditional_t<Lean, error_code_t, error_context_t>`：Lean=true 时仅存储 `error_code_t`（省去 message/payload/cause/stack），适用于热路径；Lean=false（默认）为完整模式。

**Move-Only 语义**：Lean=false 时 `result_t` 整体为 Move-Only（拷贝构造/赋值 `= delete`）；Lean=true 时若 `T` 可拷贝则 `result_t` 可拷贝。任何错误结果必须通过 `make_error` 工厂或移动构造传播，跨函数返回依赖 NRVO/移动。

### 工厂方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `make_success` | `[[nodiscard]] static result_t make_success(T value) noexcept` | 成功结果 |
| `make_error`（code + const& message） | `[[nodiscard]] static result_t make_error(error_code_t code, const std::string& message = "", utils::source_location_t loc = current()) noexcept` | 错误结果。Lean 模式下忽略 message/location，仅存 code |
| `make_error`（code + && message） | `[[nodiscard]] static result_t make_error(error_code_t code, std::string&& message, utils::source_location_t loc = current()) noexcept` | 移动 message 版本 |
| `make_error`（const context&） | `[[nodiscard]] static result_t make_error(const error_context_t& context) noexcept` | 完整模式内部 `clone()`；Lean 模式提取 code |
| `make_error`（context&&） | `[[nodiscard]] static result_t make_error(error_context_t&& context) noexcept` | 移动构造，无克隆开销 |

### 状态查询

| 方法 | 签名 | 说明 |
|------|------|------|
| `is_success` | `[[nodiscard]] bool is_success() const noexcept` | 是否成功 |
| `is_error` | `[[nodiscard]] bool is_error() const noexcept` | 是否错误 |
| `operator bool` | `explicit operator bool() const noexcept` | 等价 `is_success()` |

### 值访问

| 方法 | 签名 | 说明 |
|------|------|------|
| `value` | `[[nodiscard]] const T& value() const noexcept` | 失败返回 thread_local T{} 哨兵（另有无 const 重载） |
| `value_pointer` | `[[nodiscard]] const T* value_pointer() const noexcept` | 失败返回 nullptr（另有无 const 重载） |
| `value_or` | `[[nodiscard]] const T& value_or(const T& default_value) const noexcept` | 失败返回默认值 |
| `operator*` | `[[nodiscard]] const T& operator*() const noexcept` | 等价 value()（另有无 const 重载） |
| `operator->` | `[[nodiscard]] const T* operator->() const noexcept` | 等价 value_pointer()（另有无 const 重载） |
| `error` | `[[nodiscard]] auto error() const noexcept -> std::conditional_t<Lean, error_context_t, const error_context_t&>` | 完整模式返回 const 引用；Lean 模式返回值（临时构造仅含 code） |
| `error`（mutable） | `template <bool IsLean = Lean, typename = std::enable_if_t<!IsLean>> [[nodiscard]] error_context_t& error() noexcept` | 可变引用，仅完整模式可用（Lean 模式 SFINAE 禁用） |
| `error_code` | `[[nodiscard]] error_code_t error_code() const noexcept` | Lean 模式直接返回存储的 error_code_t；完整模式从 context 提取 |

`value()` 含 `static_assert` 要求 T 可默认构造，否则编译失败 — 此时改用 `value_pointer()` 或 `operator->`。

### 链式操作

每个链式方法有 `&` / `const&` / `&&` 三个重载（部分只有两个），下表只列一次签名，详见头文件。全部 `noexcept`，用户函数异常被 try-catch 保护并转为 fatal 错误。

| 方法 | 签名（省略重载） | 说明 |
|------|------|------|
| `and_then` | `template <typename F> [[nodiscard]] auto and_then(F&& fn) noexcept` | 成功时调用 fn(value) 返回其 result_t（`&` / `const&` / `&&` 三重载） |
| `or_else` | `template <typename F> [[nodiscard]] result_t or_else(F&& fn) noexcept` | 错误时调用 fn(error) 返回其 result_t（`&` / `&&` 两重载） |
| `map` | `template <typename F> [[nodiscard]] auto map(F&& fn) noexcept` | 成功时映射值类型（`const&` / `&&` 两重载） |
| `map_error` | `template <typename F> [[nodiscard]] result_t map_error(F&& fn) noexcept` | 错误时映射 error_context_t（`const&` / `&&` 两重载） |
| `context` | `template <typename K, typename V> ... context(K&& key, V&& value) noexcept` | 传播时附加 payload，成功时无操作（`&` 返回引用，`&&` 返回移动后对象）。Lean 模式 SFINAE 禁用 |
| `match` | `template <typename S, typename E> [[nodiscard]] auto match(S&&, E&&) const noexcept` | 模式匹配，强制同时处理两条路径 |

`match` 的 noexcept 性跟随用户回调：仅当两个回调均 noexcept 时本方法才 noexcept，否则异常会传播给调用方。

### 强制错误检查

Debug 构建下，`result_t` 析构时若处于错误状态且未被检查，触发 `assert` 提示调用方漏检错误。Release 构建下 `checked_` 标志被编译器优化掉，零开销。

以下方法会标记"已检查"：`is_success` / `is_error` / `operator bool` / `value` / `value_or` / `value_pointer` / `error` / `error_code` / `match`。

移动构造转移 `checked_` 状态（源对象标记为已检查避免其析构断言）。Lean=false 时拷贝构造已 `= delete`，无 `checked_` 转移问题；Lean=true 且 `T` 可拷贝时拷贝构造清空新对象 `checked_`，需独立检查。

### 示例

```cpp
auto r = fetch().map([](auto& d) { return d.name; });
if (auto* p = r.value_pointer()) { /* 安全使用 */ }
return inner_call().context("host", host_).context("port", port_);
```

```cpp
auto msg = r.match(
    [](const std::string& s) { return "ok: " + s; },
    [](const error_context_t& e) { return "fail: " + e.get_message(); });
```

### result_t<void, Lean> 特化

仅列与主模板的差异。构造/析构/移动 Rule of 5（拷贝 `= delete`）、`is_success`/`is_error`/`operator bool`/`make_error` 系列/`error() const`/`error_code`/`map_error`/`or_else`/`context` 均同主模板（含 Lean 模式行为）。

| 差异点 | 说明 |
|--------|------|
| 默认构造 | `result_t() noexcept` — 成功（`state_ = empty`，零 error_storage_t 构造） |
| `make_success` | `[[nodiscard]] static result_t make_success() noexcept`（无参） |
| 无 `value` / `value_or` / `value_pointer` / `operator*` / `operator->` | void 无值可读 |
| 无 mutable `error()` 重载 | — |
| 无 `map()` / `match()` | — |
| `and_then` 仅 `&` / `&&` 两个重载 | 无 const&，因 void 无值可读 |

惰性上下文设计：v4.0.0 起 `union` 仅持有 `error_storage_t` 一个成员，成功路径 `state_ = empty` 不构造任何 error 存储，零开销。失败路径才就地构造 `error_storage_t`。

```cpp
result_t<void> ok;
result_t<void> fail = result_t<void>::make_error(ERR_FAIL, "失败");
```

---

## error_context_serializer_t

错误上下文序列化器。纯静态工具类（构造/拷贝/移动/析构全部 `= delete`），提供文本 / JSON / 二进制双向转换。所有方法 `noexcept`，任何格式错误或分配失败均返回 `std::nullopt`。

### 常量

| 常量 | 值 | 说明 |
|------|------|------|
| `BINARY_MAGIC` | `0x52455345u`（"ESER" 小端） | 二进制流标识 |
| `BINARY_VERSION` | `1` | 二进制格式版本 |

### 编码

| 方法 | 签名 | 说明 |
|------|------|------|
| `to_string` | `[[nodiscard]] static std::string to_string(const error_context_t& ctx) noexcept` | 人类可读文本（含 `↳ Caused by:`） |
| `to_json` | `[[nodiscard]] static std::string to_json(const error_context_t& ctx) noexcept` | JSON（含 cause 递归字段） |
| `to_binary` | `[[nodiscard]] static std::string to_binary(const error_context_t& ctx) noexcept` | 紧凑二进制（小端序） |

### 解码

| 方法 | 签名 | 说明 |
|------|------|------|
| `from_binary` | `[[nodiscard]] static std::optional<error_context_t> from_binary(std::string_view data) noexcept` | 校验魔数与版本号，还原完整上下文 |
| `from_json` | `[[nodiscard]] static std::optional<error_context_t> from_json(std::string_view json) noexcept` | 流式解析，不构建中间 JSON 树 |

### 配置

| 方法 | 签名 | 说明 |
|------|------|------|
| `set_subsystem_module_resolver` | `static void set_subsystem_module_resolver(const i18n::i_subsystem_module_resolver_t* resolver) noexcept` | 注入自定义子系统/模块名称解析器（nullptr 重置为默认 catalog） |

反序列化的文件名与函数名由 `error_context_t` 内部字符串存储持有，保证 `file_name` 与 `source_location` 中 `const char*` 的生命周期安全。

---

## error_registry_t

错误码注册表单例。重复处理委托 `duplicate_policy_handler_t`，子系统/模块名称注册与查询已迁移至 `subsystem_module_catalog_t`。

| 方法 | 签名 | 说明 |
|------|------|------|
| `instance` | `static error_registry_t& instance() noexcept` | 单例（`std::call_once` + `std::once_flag`） |

### 注册

| 方法 | 签名 | 说明 |
|------|------|------|
| `register_error` | `void register_error(error_code_t code, std::string_view name, std::string_view description) noexcept` | 单个注册 |
| `register_errors` | `[[nodiscard]] size_t register_errors(const std::vector<error_code_t>& codes, const std::vector<std::string_view>& names, const std::vector<std::string_view>& descriptions) noexcept` | 批量注册，返回成功数量；数组长度不一致返回 0 |
| `unregister_error` | `void unregister_error(error_code_t code) noexcept` / `void unregister_error(std::string_view name) noexcept` | 按 code / name 注销 |
| `unregister_module` | `void unregister_module(module_group_id_t) noexcept` | 注销模块组所有错误码 |
| `unregister_all` | `void unregister_all() noexcept` | 清空 |

### 查询

| 方法 | 签名 | 说明 |
|------|------|------|
| `is_registered` | `[[nodiscard]] bool is_registered(error_code_t code) const noexcept` | 是否已注册 |
| `get_info` | `[[nodiscard]] std::optional<error_metadata_t> get_info(error_code_t code) const noexcept` | 元数据副本（强一致，`shared_lock`） |
| `get_errors_by_subsystem` | `[[nodiscard]] std::vector<error_metadata_t> get_errors_by_subsystem(uint16_t subsys_id) const noexcept` | 子系统下所有错误码 |
| `get_errors_by_module` | `[[nodiscard]] std::vector<error_metadata_t> get_errors_by_module(module_group_id_t) const noexcept` | 模块下所有错误码 |
| `find_by_name` | `[[nodiscard]] std::optional<error_code_t> find_by_name(std::string_view name) const noexcept` | 按名称查找 |

### 缓存

| 方法 | 签名 | 说明 |
|------|------|------|
| `get_info_cached` | `std::optional<error_metadata_t> get_info_cached(error_code_t code) const noexcept` | thread_local 环形缓存（容量 16），命中零锁开销 |
| `get_epoch` | `[[nodiscard]] uint64_t get_epoch() const noexcept` | 注册表纪元（acquire 序） |
| `invalidate_metadata_cache` | `void invalidate_metadata_cache() const noexcept` | 清除当前线程缓存（仅测试用） |

`get_info()` vs `get_info_cached()`：

| 路径 | 锁开销 | 一致性 | 适用 |
|------|:---:|:---:|------|
| `get_info()` | `shared_lock` | 强一致 | 管理工具、偶尔查询 |
| `get_info_cached()` | 命中时零锁 | 纪元失效后最终一致 | `error_context_t` 构造等热路径 |

缓存检测到纪元变化（acquire 序）后整体失效重建，同时记录"已注册"与"未注册"结果避免重复加锁查询。任何 `register_*` / `unregister_*` 调用会 `bump_epoch_()`（release 序）。查询路径选择决策树详见 [决策树 · 2](../decision_tree.md#2-错误码元数据查询路径选择)。

### 重复策略

| 方法 | 签名 | 说明 |
|------|------|------|
| `set_duplicate_policy` | `void set_duplicate_policy(duplicate_policy_t) noexcept` | 设置策略（转发至 handler） |
| `get_duplicate_policy` | `duplicate_policy_t get_duplicate_policy() const noexcept` | 获取当前策略 |
| `set_duplicate_warn_callback` | `void set_duplicate_warn_callback(duplicate_warn_callback_t) noexcept` | 设置警告回调（nullptr 清除） |
| `get_duplicate_warn_callback` | `const duplicate_warn_callback_t& get_duplicate_warn_callback() const noexcept` | 获取当前回调 |

#### duplicate_policy_t

| 枚举值 | 说明 |
|--------|------|
| `skip` | 静默跳过（默认） |
| `overwrite` | 覆盖已有定义 |
| `warn` | 跳过但记录警告（触发回调） |

#### error_metadata_t

错误码元数据信息，仅含值语义字段，可复制，线程安全（注册表返回副本）。

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `name` | `std::string` | `{}` | 错误码宏名称 |
| `description` | `std::string` | `{}` | 错误码描述文本 |
| `module_id` | `uint16_t` | `0` | 模块 ID |
| `error_number` | `uint16_t` | `0` | 错误编号 |
| `level` | `error_level_t` | `info` | 错误等级 |

子系统/模块名称注册与查询已迁移至 `subsystem_module_catalog_t`。

---

## error_builder_t

纯静态工具类（构造/拷贝/移动全部 `= delete`）。保留两个有独特价值的工厂方法，其他场景使用 `error_code_t` 便捷构造。

| 方法 | 签名 | 说明 |
|------|------|------|
| `make_error_code` | `template <typename SubSystemEnum, typename ModuleEnum, typename = std::enable_if_t<is_uint16_enum_v<SubSystemEnum> && is_uint16_enum_v<ModuleEnum>>> [[nodiscard]] static constexpr error_code_t make_error_code(error_level_t level, domain::system_domain_t system, SubSystemEnum subsystem, ModuleEnum module, uint16_t number) noexcept` | 枚举模板版本，编译期类型安全（SFINAE 约束枚举大小不超过 2 字节），防止 subsystem/module ID 传反 |
| `from_raw` | `[[nodiscard]] static constexpr error_code_t from_raw(code_t code) noexcept` | 从 64 位原始码恢复（语义明确表达"反序列化"意图） |

```cpp
enum class subsys_t : uint16_t { db_conn = 1 };
enum class module_t : uint16_t { timeout = 2 };
auto code = error_builder_t::make_error_code(
    error_level_t::error, system_domain_t::database,
    subsys_t::db_conn, module_t::timeout, 0x0001);
error_code_t restored = error_builder_t::from_raw(recv_from_network());
```

---

## error_exception_t

将 `error_context_t` 封装为可抛出异常，继承 `std::exception`。**v4.0.0 起 `error_context_t` 为 Move-Only，无法直接满足 C++ 异常的可拷贝要求，故内部用 `std::shared_ptr<const error_context_t>` 持有**：构造时 `make_shared` 转移所有权（零深拷贝），拷贝异常仅增加引用计数。构造时通过 `error_context_serializer_t::to_string` 缓存错误详情字符串，`what()` 在异常传播期间返回稳定指针。

| 方法 | 签名 | 说明 |
|------|------|------|
| 构造 | `explicit error_exception_t(error_context_t context) noexcept` | 按值接收以支持移动，内部 `make_shared` 转移所有权，缓存 `to_string` 结果 |
| `what` | `const char* what() const noexcept override` | 返回缓存的完整错误详情 |
| `context` | `[[nodiscard]] const error_context_t& context() const noexcept` | 原始错误上下文（未持有时返回空哨兵） |
| `code` | `[[nodiscard]] error_code_t code() const noexcept` | 原始错误码（未持有时返回默认成功码） |

拷贝构造 `= default`、移动构造 `noexcept = default`；拷贝/移动赋值均 `= delete`。

---

## error_level_t

错误等级强类型枚举及配套 constexpr 转换函数。全部函数 `constexpr noexcept`，可用于编译期常量与日志过滤模板参数。

### 枚举值

| 枚举值 | 整数值 | 说明 |
|--------|:---:|------|
| `debug` | 0 | 调试 |
| `info` | 1 | 信息 |
| `warn` | 2 | 警告 |
| `error` | 3 | 错误 |
| `fatal` | 4 | 致命错误 |

### 转换函数

| 函数 | 签名 | 说明 |
|------|------|------|
| `to_int` | `[[nodiscard]] constexpr uint8_t to_int(error_level_t level) noexcept` | 转整数 |
| `is_valid` | `[[nodiscard]] constexpr bool is_valid(uint8_t level) noexcept` | 整数是否有效（≤ fatal） |
| `from_int` | `[[nodiscard]] constexpr error_level_t from_int(uint8_t level) noexcept` | 整数转枚举（非法值回退 fatal） |
| `to_string` | `[[nodiscard]] constexpr const char* to_string(error_level_t level) noexcept` | 转字符串（非法值返回 "unknown"） |
| `from_string` | `[[nodiscard]] constexpr error_level_t from_string(const char* str) noexcept` | 字符串转枚举（未知返回 info） |
| `next_level` | `[[nodiscard]] constexpr error_level_t next_level(error_level_t level) noexcept` | 下一级（越界回退 fatal） |
| `prev_level` | `[[nodiscard]] constexpr error_level_t prev_level(error_level_t level) noexcept` | 上一级 |
| `should_log` | `[[nodiscard]] constexpr bool should_log(error_level_t current, error_level_t min_level) noexcept` | 当前等级 ≥ 最小等级 |

---

## DEFINE_ERROR_CODE

定义 `constexpr` 错误码常量并自动注册到 `error_registry_t`。利用 C++ 静态初始化在 `main()` 之前完成注册，无需手动调用。

### 参数

| 参数 | 说明 |
|------|------|
| `NAME` | 错误码宏名（如 `ERR_DB_FAIL`） |
| `LEVEL` | `error_level_t::xxx` |
| `SYSTEM` | `system_domain_t::xxx` |
| `SUBSYS` | 子系统 ID（`uint16_t`） |
| `MODULE` | 模块 ID（`uint16_t`） |
| `NUMBER` | 错误编号（`uint16_t`） |
| `DESC` | 错误描述（`const char*`） |
| `SUBSYS_NAME` | 子系统名称（已废弃，保留向后兼容；请通过 `subsystem_module_catalog_t` 注册） |
| `MODULE_NAME` | 模块名称（已废弃，保留向后兼容；请通过 `subsystem_module_catalog_t` 注册） |

```cpp
DEFINE_ERROR_CODE(ERR_DB_FAIL, error_level_t::error, system_domain_t::database,
    1, 1, 0x0010, "数据库操作失败", "数据库服务", "连接管理");
```

`constexpr error_code_t` 常量为编译期决议，无 SIOF 风险；但请勿在其它 TU 的静态初始化代码中查询注册表（跨 TU 动态初始化顺序未指定）。运行时查询（如 `error_context_t` 构造）发生在 `main()` 之后所有静态初始化完成时，不受影响。

---

## i_error_notifier_t

错误通知器抽象接口。解耦 core 层对 plugin 层的反向依赖：core 层通过此接口通知错误事件，plugin 层提供具体实现（如 `plugin_registry_t`）。遵循依赖倒置原则。

| 方法 | 签名 | 说明 |
|------|------|------|
| 析构 | `virtual ~i_error_notifier_t() noexcept = default` | 虚析构 |
| `notify_error` | `virtual void notify_error(const error_context_t& context) noexcept = 0` | 同步通知（sync 模式） |
| `enqueue_notification` | `virtual void enqueue_notification(const error_context_t& context) noexcept = 0` | 异步入队（async_queue 模式） |
| `enqueue_deferred_notification` | `virtual void enqueue_deferred_notification(const error_context_t& context) noexcept = 0` | 累积到线程本地缓冲，flush 时批量通知（sync_deferred 模式） |

实现类必须保证所有方法 noexcept 安全，插件回调抛出的异常应在实现内部捕获并记录，不向外传播。

---

## error_context_initializer_t

错误上下文初始化器。纯静态工具类（构造/拷贝/移动/析构全部 `= delete`）。在 `error_context_t` 构造时根据全局配置完成错误码校验、堆栈捕获、源位置记录和插件通知。通过 `error_context_t` 的 friend 声明访问其私有成员。插件通知通过 `i_error_notifier_t` 抽象接口完成，core 层不直接依赖 plugin 层。

| 方法 | 签名 | 说明 |
|------|------|------|
| `set_error_notifier` | `static void set_error_notifier(i_error_notifier_t* notifier) noexcept` | 注入通知器实现（nullptr 清除）。非线程安全，预期在初始化阶段调用 |
| `get_error_notifier` | `[[nodiscard]] static i_error_notifier_t* get_error_notifier() noexcept` | 获取当前通知器（未设置返回 nullptr） |
| `initialize` | `static void initialize(error_context_t& context) noexcept` | 执行运行时特性初始化：校验 → 堆栈 → 源位置 → 通知。成功码上下文由调用方自行跳过 |
