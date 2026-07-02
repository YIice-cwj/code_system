# Utils 层 API

`error_system::utils`

通用工具层，提供异步队列、字符串处理、JSON 解析、文件操作、堆栈跟踪、源位置与格式化等能力。

---

## async_queue_t\<T, Processor\>

头文件：`error_system/utils/async_queue.h`

异步工作队列，单生产者-单消费者模式。首次 `enqueue()` 启动工作线程，析构自动 `join()`。`T` 必须可移动构造；`Processor` 必须可调用 `void(T&)` 且 `noexcept`。不可拷贝、不可移动。

类型别名：`value_type_t = T`，`processor_t = Processor`。

| 方法 | 签名 | 说明 |
|------|------|------|
| `async_queue_t` | `explicit async_queue_t(processor_t processor) noexcept` | 注入处理器 |
| `enqueue` | `bool enqueue(value_type_t item) noexcept` | 入队，首次调用自动启动工作线程 |
| `set_max_size` | `void set_max_size(size_t size) noexcept` | 最大容量，0 = 无限制 |
| `max_size` | `[[nodiscard]] size_t max_size() const noexcept` | 最大容量 |
| `size` | `[[nodiscard]] size_t size() const noexcept` | 当前队列大小 |
| `empty` | `[[nodiscard]] bool empty() const noexcept` | 是否为空 |

| 特性 | 说明 |
|------|------|
| 自动生命周期 | 首次 `enqueue()` 启动，析构 `join()` |
| 死锁安全 | 先设 `running_=false`，再 `notify_all`，再 `join` |
| 异常隔离 | 处理器异常不退出工作线程 |
| 背压控制 | 队列满拒绝入队 |
| 零 `std::function` | `Processor` 模版参数，编译期确定调用 |

```cpp
async_queue_t<int, decltype([](int& v) noexcept { process(v); })> queue(
    [](int& v) noexcept { process(v); });
queue.enqueue(42);
queue.set_max_size(1000);
```

---

## string_utils_t

头文件：`error_system/utils/string_utils.h`

字符串处理工具类（不可实例化，全部为静态方法）。格式化功能已迁移至 `string_format_t`；JSON 转义功能已迁移至 `json_serializer_t`。

### 哈希与检测

| 方法 | 签名 | 说明 |
|------|------|------|
| `hash` | `[[nodiscard]] static constexpr uint64_t hash(std::string_view string) noexcept` | FNV-1a 哈希 |
| `hash_limit` | `[[nodiscard]] static constexpr uint64_t hash_limit(std::string_view string, size_t max_length = 128) noexcept` | 限制哈希长度 |
| `starts_with` | `[[nodiscard]] static constexpr bool starts_with(std::string_view string, std::string_view prefix) noexcept` | 前缀检测 |
| `ends_with` | `[[nodiscard]] static constexpr bool ends_with(std::string_view string, std::string_view suffix) noexcept` | 后缀检测 |

### 解析与变换

| 方法 | 签名 | 说明 |
|------|------|------|
| `parse_number` | `template <typename T> static std::optional<T> parse_number(std::string_view string) noexcept` | `from_chars` 解析数字 |
| `replace_all` | `[[nodiscard]] static std::string replace_all(std::string string, std::string_view from, std::string_view to) noexcept` | 替换所有子串 |
| `split` | `[[nodiscard]] static std::vector<std::string_view> split(std::string_view string, std::string_view delimiter) noexcept` | 分割字符串 |
| `join` | `[[nodiscard]] static std::string join(const std::vector<std::string_view>& tokens, std::string_view delimiter) noexcept` | 合并字符串 |
| `trim` | `[[nodiscard]] static std::string_view trim(std::string_view string) noexcept` | 移除首尾空白 |
| `to_lower` | `[[nodiscard]] static std::string to_lower(std::string_view string) noexcept` | 转小写 |
| `to_upper` | `[[nodiscard]] static std::string to_upper(std::string_view string) noexcept` | 转大写 |

```cpp
string_utils_t::hash("hello");                  // FNV-1a 哈希
string_utils_t::split("a,b,c", ",");             // {"a", "b", "c"}
string_utils_t::replace_all("a b", "b", "c");   // "a c"
string_utils_t::parse_number<int>("42");         // optional{42}
```

---

## string_format_t

头文件：`error_system/utils/string_format.h`

`{}` 占位符格式化工具，`error_context_t` 消息格式化使用。支持算术类型、指针、`bool`、`char`，以及含 `to_string()` 成员或全局函数的自定义类型。`{{` / `}}` 转义为字面 `{` / `}`。

| 方法 | 签名 | 说明 |
|------|------|------|
| `format` | `template <typename... Args> [[nodiscard]] static std::string format(std::string_view format_str, Args&&... args) noexcept` | 占位符替换 |

```cpp
string_format_t::format("用户 {} 登录失败，重试 {} 次", "alice", 3);
// "用户 alice 登录失败，重试 3 次"
```

---

## json_dict_t

头文件：`error_system/utils/json_utils.h`

JSON 解析与点路径访问。默认可拷贝、可移动。仅支持扁平 / 嵌套的字符串键值对，不支持数字、布尔、数组等非字符串值。生产环境如需完整 JSON 解析请使用 nlohmann/json 等第三方库。

| 方法 | 签名 | 说明 |
|------|------|------|
| `operator[]` | `[[nodiscard]] std::optional<std::string> operator[](const std::string& key) const noexcept` | 键访问 |
| `get_value` | `[[nodiscard]] std::optional<std::string> get_value(const std::string& key) const noexcept` | 获取值，支持 `"a.b"` 点路径 |
| `get_value_or` | `[[nodiscard]] std::string get_value_or(const std::string& key, const std::string& default_value) const noexcept` | 获取值或默认值 |
| `contains` | `[[nodiscard]] bool contains(const std::string& key) const noexcept` | 是否包含键 |
| `empty` | `[[nodiscard]] bool empty() const noexcept` | 是否为空 |
| `size` | `[[nodiscard]] size_t size() const noexcept` | 键值对数量 |
| `from_file` | `[[nodiscard]] static std::optional<json_dict_t> from_file(const std::filesystem::path& json_path) noexcept` | 从文件加载 |
| `parse` | `[[nodiscard]] static std::optional<json_dict_t> parse(const std::string& json_content) noexcept` | 解析 JSON 字符串 |

```cpp
auto dict = json_dict_t::parse(R"({"user":{"name":"Alice"}})");
dict->get_value("user.name");      // "Alice"
dict->get_value_or("code", "0");   // "0"
```

---

## json_serializer_t

头文件：`error_system/utils/json_utils.h`

JSON 序列化辅助工具（不可实例化）。

| 方法 | 签名 | 说明 |
|------|------|------|
| `escape_json` | `[[nodiscard]] static std::string escape_json(std::string_view value) noexcept` | 转义 JSON 字符串 |

```cpp
json_serializer_t::escape_json(R"(a"b\c)");  // "a\"b\\c"
```

---

## json_lexer_t

头文件：`error_system/utils/json_lexer.h` ｜ 命名空间：`error_system::utils::detail`

`json_dict_t` 的词法分析基础，支持 RFC 8259 的 UTF-16 代理对解析（`\uD83D\uDE00` → 4 字节 UTF-8），孤立代理区码点静默丢弃。默认可拷贝、可移动。

### token_type_t 枚举

| 枚举值 | 说明 |
|--------|------|
| `string` | 字符串（键或值） |
| `number` | 数字字面量 |
| `true_literal` | `true` |
| `false_literal` | `false` |
| `null_literal` | `null` |
| `colon` | 冒号 `:` |
| `comma` | 逗号 `,` |
| `left_brace` | 左大括号 `{` |
| `right_brace` | 右大括号 `}` |
| `left_bracket` | 左中括号 `[` |
| `right_bracket` | 右中括号 `]` |
| `eof` | 文件结束标识 |
| `invalid` | 无效字符或错误 |

### token_t 结构

| 字段 | 类型 | 默认值 |
|------|------|--------|
| `type` | `token_type_t` | `token_type_t::eof` |
| `value` | `std::string` | — |

### 方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `json_lexer_t` | `explicit json_lexer_t(std::string_view json_text) noexcept` | 构造词法分析器 |
| `next` | `[[nodiscard]] token_t next() noexcept` | 获取下一个 token |

---

## file_utils_t

头文件：`error_system/utils/file_utils.h`

跨平台文件操作工具（不可实例化）。`read_file()` 内置文件大小上限保护（`MAX_READ_FILE_SIZE = 64 MB`），防止 OOM 攻击，超过阈值返回 `std::nullopt`。

| 常量 | 值 |
|------|---:|
| `MAX_READ_FILE_SIZE` | `64 * 1024 * 1024` (64 MB) |

| 方法 | 签名 | 说明 |
|------|------|------|
| `read_file` | `[[nodiscard]] static std::optional<std::string> read_file(const std::filesystem::path& path) noexcept` | 读取文件，超限返回 `nullopt` |
| `write_file` | `[[nodiscard]] static bool write_file(const std::filesystem::path& path, const std::string& content) noexcept` | 写入文件 |
| `create_file` | `[[nodiscard]] static bool create_file(const std::filesystem::path& path) noexcept` | 创建文件 |
| `delete_file` | `[[nodiscard]] static bool delete_file(const std::filesystem::path& path) noexcept` | 删除文件 |
| `force_delete_file` | `[[nodiscard]] static bool force_delete_file(const std::filesystem::path& path) noexcept` | 强制删除 |
| `file_exists` | `[[nodiscard]] static bool file_exists(const std::filesystem::path& path) noexcept` | 文件是否存在 |
| `dir_exists` | `[[nodiscard]] static bool dir_exists(const std::filesystem::path& path) noexcept` | 目录是否存在 |

```cpp
auto content = file_utils_t::read_file("config.json");
file_utils_t::write_file("out.txt", "Hello!");
```

---

## stack_trace_utils_t

头文件：`error_system/utils/stack_trace_utils.h`

跨平台堆栈跟踪工具（不可实例化）。

| 方法 | 签名 | 说明 |
|------|------|------|
| `generate` | `[[nodiscard]] static std::vector<std::string> generate(int skip_frames = 1, int max_frames = 16) noexcept` | 抓取当前线程调用栈 |

| 平台 | 实现 |
|------|------|
| Linux / macOS | `backtrace()` / `backtrace_symbols()` + cxxabi |
| Windows | `StackWalk64()` + `SymFromAddr` |

```cpp
auto frames = stack_trace_utils_t::generate(2, 8);
```

---

## source_location_t

头文件：`error_system/utils/source_location.h`

源位置封装，`error_context_t` 构造时通过 `located_code_t` 隐式转换自动捕获调用者位置。默认可拷贝、可移动。通过默认参数在调用点展开 `__builtin_FILE()`，捕获的是调用者位置而非库内部位置。

### 自由函数

| 函数 | 签名 | 说明 |
|------|------|------|
| `extract_short_filename` | `[[nodiscard]] constexpr const char* extract_short_filename(const char* path) noexcept` | 提取路径中的文件名部分 |

### 方法

| 方法 | 签名 | 说明 |
|------|------|------|
| `source_location_t` | `constexpr source_location_t() noexcept = default` | 默认构造 |
| `source_location_t` | `constexpr source_location_t(const char* file, const char* func, uint32_t line) noexcept` | 从字符串构造（反序列化用） |
| `current` | `[[nodiscard]] static constexpr source_location_t current(const char* file = __builtin_FILE(), const char* func = __builtin_FUNCTION(), uint32_t line = __builtin_LINE()) noexcept` | 获取当前源位置 |
| `file_name` | `[[nodiscard]] constexpr const char* file_name() const noexcept` | 源文件路径 |
| `function_name` | `[[nodiscard]] constexpr const char* function_name() const noexcept` | 函数名 |
| `line` | `[[nodiscard]] constexpr uint32_t line() const noexcept` | 行号 |

---

## error_formatter

头文件：`error_system/core/error_formatter.h` ｜ 命名空间：`error_system::core`

`error_context_t` 的 `operator<<` 输出流支持。

| 方法 | 签名 | 说明 |
|------|------|------|
| `operator<<` | `[[nodiscard]] std::ostream& operator<<(std::ostream& stream, const error_context_t& context) noexcept` | 序列化为可读文本并写入流 |

```
[Location: main.cc:42 @ main] [Sign: Error Level: error, System: database, 数据库服务 / 连接管理]
Code: 1 (ERR_DB_TIMEOUT) - 数据库连接超时
```
