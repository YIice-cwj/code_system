# Utils 层 API 文档

> 命名空间：`error_system::utils`

Utils 层提供字符串处理、JSON 解析和文件操作等通用工具，被系统各层广泛使用。

---

## string_utils_t

**头文件**：`error_system/utils/string_utils.h`

静态工具类，所有方法均为 `static`，不可实例化。

### hash / hash_limit

编译期 FNV-1a 哈希，用于 `constexpr switch` 的字符串分发。

```cpp
// 编译期哈希
constexpr uint64_t hash(std::string_view str) noexcept;

// 限制长度后哈希（超出则截断）
constexpr uint64_t hash_limit(std::string_view str, size_t max_length = 128) noexcept;

// 用法：switch 中进行字符串匹配
switch (string_utils_t::hash(input)) {
    case string_utils_t::hash("debug"): return error_level_t::debug;
    case string_utils_t::hash("error"): return error_level_t::error;
}
```

### format

轻量字符串格式化，使用 `{}` 作为占位符，支持任意可流输出类型。

```cpp
template<typename... Args>
static std::string format(std::string_view fmt, const Args&... args);

// 示例
std::string msg = string_utils_t::format(
    "[Level: {}, Code: {}]", "error", 404
);
// → "[Level: error, Code: 404]"
```

### starts_with / ends_with

```cpp
constexpr bool starts_with(std::string_view str, std::string_view prefix) noexcept;
constexpr bool ends_with(std::string_view str, std::string_view suffix) noexcept;
```

### parse_number

将字符串解析为数字，失败返回 `std::nullopt`。

```cpp
template<typename T>
static std::optional<T> parse_number(std::string_view str) noexcept;

auto n = string_utils_t::parse_number<int>("42");  // optional(42)
auto x = string_utils_t::parse_number<int>("abc"); // nullopt
```

### replace_all / split / join / trim

```cpp
// 替换所有出现
static std::string replace_all(std::string str, std::string_view from, std::string_view to) noexcept;

// 按分隔符分割（返回 string_view 视图，零拷贝）
static std::vector<std::string_view> split(std::string_view str, std::string_view delim) noexcept;

// 合并
static std::string join(const std::vector<std::string_view>& tokens, std::string_view delim) noexcept;

// 去除首尾空白
static std::string_view trim(std::string_view str) noexcept;
```

### to_lower / to_upper

```cpp
static std::string to_lower(std::string_view str) noexcept;
static std::string to_upper(std::string_view str) noexcept;
```

---

## json_dict_t

**头文件**：`error_system/utils/json_utils.h`

轻量 JSON 字典，支持点分隔路径的嵌套键访问，主要用于 i18n 字典加载。

```cpp
// 从文件加载
static std::optional<json_dict_t> from_file(const std::string& path);

// 从字符串解析
static std::optional<json_dict_t> parse(std::string_view json);

// 获取值，key 支持点分隔嵌套路径
std::optional<std::string> get_value(const std::string& key) const;

// 获取值，若不存在则返回 default_value
std::optional<std::string> get_value_or(const std::string& key,
                                         const std::string& default_value) const;

// 判断字典是否为空
bool empty() const noexcept;

// 示例
auto dict = json_dict_t::from_file("zh_cn.json");
std::string text = dict->get_value_or("domain.database", "database").value();
```

---

## file_utils

**头文件**：`error_system/utils/file_utils.h`

跨平台文件读写工具函数。

```cpp
// 读取整个文件内容
std::optional<std::string> read_file(const std::string& path);

// 写入文件（覆盖）
bool write_file(const std::string& path, std::string_view content);
```
