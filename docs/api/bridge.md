# Bridge 层 API

与 C ABI / `std::error_code` / `errno` 的互操作层，使 `error_code_t` 可被 Python/Go/Rust FFI 和 C++ 标准库 `<system_error>` 消费。

## C ABI 导出（`error_system::abi`）

`error_code_t` 的 C 兼容句柄接口。C++ 调用方直接使用 inline 函数；C/FFI 调用方链接 `src/bridge/c_abi_export.cc` 中的 `extern "C"` 符号。

### 句柄管理

| 函数 | 签名 | 说明 |
|------|------|------|
| `create_handle` | `handle_t create_handle(code_t raw_code) noexcept` | 堆分配错误码，返回句柄；失败返回 nullptr |
| `free_handle` | `void free_handle(handle_t handle) noexcept` | 释放句柄，nullptr 安全 |
| `to_handle` | `handle_t to_handle(const void* opaque) noexcept` | void* → handle_t 边界转换 |
| `from_handle` | `const void* from_handle(handle_t handle) noexcept` | handle_t → void* 边界转换 |

### 字段访问

| 函数 | 返回类型 | 说明 |
|------|---------|------|
| `get_raw` | `code_t` | 原始 64 位码 |
| `is_error` | `int` | 1=错误，0=成功/无效 |
| `is_success` | `int` | 1=成功，0=错误/无效 |
| `get_level` | `uint8_t` | 错误等级 |
| `get_system` | `uint8_t` | 系统域 |
| `get_subsystem` | `uint16_t` | 子系统 ID |
| `get_module` | `uint16_t` | 模块 ID |
| `get_number` | `uint16_t` | 错误编号 |
| `is_retryable` | `int` | 1=可重试 |
| `is_transient` | `int` | 1=瞬态错误 |
| `get_message` | `int` | 十六进制码字符串写入 buf，返回字节数 |

所有函数接受 `handle_t`，nullptr 时返回 0。

### extern "C" 接口

C 调用方通过以下符号访问（`const void*` 即句柄）：

```c
const void* error_system_code_create(uint64_t raw_code);
void error_system_code_handle_free(const void* handle);
uint64_t error_system_code_raw(const void* handle);
int error_system_code_is_error(const void* handle);
int error_system_code_is_success(const void* handle);
uint8_t error_system_code_level(const void* handle);
uint8_t error_system_code_system(const void* handle);
uint16_t error_system_code_subsystem(const void* handle);
uint16_t error_system_code_module(const void* handle);
uint16_t error_system_code_number(const void* handle);
int error_system_code_is_retryable(const void* handle);
int error_system_code_is_transient(const void* handle);
int error_system_code_message(const void* handle, char* buf, int buf_size);
```

Python ctypes 示例：

```python
lib = ctypes.CDLL("liberror_system.so")
h = lib.error_system_code_create(0x0000FFFE00010010)
buf = ctypes.create_string_buffer(64)
lib.error_system_code_message(h, buf, 64)
lib.error_system_code_handle_free(h)
```

## std::error_code 桥接（`error_system::bridge`）

`error_code_t` 与 `std::error_code` / `errno` 的双向转换。

### error_system_category_t

自定义 `std::error_category` 子类，承载 `error_code_t` 的结构化信息。

| 方法 | 说明 |
|------|------|
| `name()` | 返回 `"error_system"` |
| `message(ev)` | 返回 `"error_system:0x..."` 十六进制表示 |
| `error_system_category()` | 获取单例引用（跨 TU 稳定） |

### 转换函数

| 函数 | 签名 | 说明 |
|------|------|------|
| `from_errno` | `error_code_t from_errno(int err_no) noexcept` | errno → error_code_t（system 域，number 携带 errno 值；EAGAIN/ETIMEDOUT/EINTR 标记 retryable） |
| `to_errno` | `int to_errno(error_code_t code) noexcept` | error_code_t → errno 值（成功返回 0；system 域反解 number；其他返回 EIO） |
| `to_std_error_code` | `std::error_code to_std_error_code(error_code_t) noexcept` | error_code_t → std::error_code（成功码返回默认构造） |
| `from_std_error_code` | `error_code_t from_std_error_code(const std::error_code&) noexcept` | std::error_code → error_code_t（error_system_category 反解 identity；generic/system category 走 errno 桥接） |
| `throw_system_error` | `[[noreturn]] void throw_system_error(error_code_t, std::string_view what)` | 抛出 `std::system_error`（库边界使用） |

```cpp
// errno → error_code_t
auto code = bridge::from_errno(EINVAL);

// error_code_t → std::error_code
std::error_code ec = bridge::to_std_error_code(my_code);
if (ec) std::cerr << ec.message();

// 往返转换
auto restored = bridge::from_std_error_code(ec);
```
