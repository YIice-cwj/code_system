# Mapping 层 API

`error_system::mapping`

错误码映射模块，将内部 `error_code_t` 翻译为标准 HTTP / gRPC 状态码，便于在 Web/RPC 边界统一对外暴露错误。所有方法均为 `constexpr`，可在编译期求值，零运行时开销。

---

## http_status_t

HTTP 状态码包装类，仅包含错误系统映射所需的状态码子集（非完整 HTTP 标准覆盖）。枚举值与 HTTP 标准（RFC 7231）一致。

### value_t 枚举值

| 枚举值 | 数值 | 枚举值 | 数值 |
|--------|:---:|--------|:---:|
| ok | 200 | payload_too_large | 413 |
| bad_request | 400 | uri_too_long | 414 |
| unauthorized | 401 | too_many_requests | 429 |
| forbidden | 403 | internal_server_error | 500 |
| not_found | 404 | not_implemented | 501 |
| method_not_allowed | 405 | bad_gateway | 502 |
| request_timeout | 408 | service_unavailable | 503 |
| conflict | 409 | gateway_timeout | 504 |
| gone | 410 | | |

### 构造函数

| 构造函数 | 签名 | 说明 |
|----------|------|------|
| 默认构造 | `constexpr http_status_t() noexcept = default` | 默认 ok |
| 从枚举 | `constexpr explicit http_status_t(value_t value) noexcept` | 从枚举值构造 |
| 从整数 | `constexpr explicit http_status_t(uint16_t code) noexcept` | 非法值回退 internal_server_error |

### 方法

| 方法 | 签名 | 说明 |
|------|------|------|
| value | `[[nodiscard]] constexpr value_t value() const noexcept` | 获取底层枚举值 |
| to_int | `[[nodiscard]] constexpr uint16_t to_int() const noexcept` | 转换为整数 |
| c_str | `[[nodiscard]] const char* c_str() const noexcept` | 如 "Service Unavailable" |
| from_int | `[[nodiscard]] static constexpr http_status_t from_int(int value) noexcept` | 未知返回 internal_server_error |
| is_valid | `[[nodiscard]] static constexpr bool is_valid(int value) noexcept` | 检查是否为已知状态码 |
| is_success | `[[nodiscard]] constexpr bool is_success() const noexcept` | 2xx |
| is_client_error | `[[nodiscard]] constexpr bool is_client_error() const noexcept` | 4xx |
| is_server_error | `[[nodiscard]] constexpr bool is_server_error() const noexcept` | 5xx |
| operator== | `constexpr bool operator==(http_status_t other) const noexcept` | 相等比较 |
| operator!= | `constexpr bool operator!=(http_status_t other) const noexcept` | 不等比较 |

**使用示例**

```cpp
http_status_t s{http_status_t::value_t::service_unavailable};
s.to_int();              // → 503
s.c_str();               // → "Service Unavailable"
s.is_server_error();     // → true
http_status_t::from_int(503);  // → service_unavailable
```

---

## grpc_status_t

gRPC 状态码包装类，数值与 `grpc::StatusCode` 一致，避免引入 gRPC 依赖。

### value_t 枚举值

| 枚举值 | 数值 | 枚举值 | 数值 |
|--------|:---:|--------|:---:|
| ok | 0 | failed_precondition | 9 |
| cancelled | 1 | aborted | 10 |
| unknown | 2 | out_of_range | 11 |
| invalid_argument | 3 | unimplemented | 12 |
| deadline_exceeded | 4 | internal | 13 |
| not_found | 5 | unavailable | 14 |
| already_exists | 6 | data_loss | 15 |
| permission_denied | 7 | unauthenticated | 16 |
| resource_exhausted | 8 | | |

### 构造函数

| 构造函数 | 签名 | 说明 |
|----------|------|------|
| 默认构造 | `constexpr grpc_status_t() noexcept = default` | 默认 ok |
| 从枚举 | `constexpr explicit grpc_status_t(value_t value) noexcept` | 从枚举值构造 |

### 方法

| 方法 | 签名 | 说明 |
|------|------|------|
| value | `[[nodiscard]] constexpr value_t value() const noexcept` | 获取底层枚举值 |
| to_int | `[[nodiscard]] constexpr uint16_t to_int() const noexcept` | 转换为整数 |
| c_str | `[[nodiscard]] const char* c_str() const noexcept` | 如 "INTERNAL" |
| from_int | `[[nodiscard]] static constexpr grpc_status_t from_int(int value) noexcept` | 越界返回 unknown |
| is_valid | `[[nodiscard]] static constexpr bool is_valid(int value) noexcept` | 0..16 |
| operator== | `constexpr bool operator==(grpc_status_t other) const noexcept` | 相等比较 |
| operator!= | `constexpr bool operator!=(grpc_status_t other) const noexcept` | 不等比较 |

**使用示例**

```cpp
grpc_status_t s{grpc_status_t::value_t::internal};
s.c_str();               // → "INTERNAL"
s.to_int();              // → 13
grpc_status_t::is_valid(13);  // → true
grpc_status_t::from_int(13); // → internal
```

---

## status_mapper_t

错误码到 HTTP / gRPC 状态码的映射器，纯函数工具类，不可实例化。

### 映射策略

`retryable` / `transient` 优先映射为 `503 / UNAVAILABLE`，提示客户端重试；其余按错误等级与系统域细分。

| 错误等级 | 系统域 | HTTP | gRPC |
|------|------|:---:|:---:|
| 成功码 / debug·info·warn | — | 200 OK | OK |
| error | application | 400 Bad Request | INVALID_ARGUMENT |
| error | third_party | 502 Bad Gateway | UNAVAILABLE |
| error | database | 503 Service Unavailable | DATA_LOSS |
| error | middleware | 503 Service Unavailable | UNAVAILABLE |
| error | system / none | 500 Internal Server Error | INTERNAL |
| fatal | — | 500 Internal Server Error | INTERNAL |
| 任意（retryable/transient） | — | 503 Service Unavailable | UNAVAILABLE |

### API

| 方法 | 签名 | 说明 |
|------|------|------|
| to_http_status | `[[nodiscard]] static constexpr http_status_t to_http_status(error_code_t code) noexcept` | 映射为 HTTP 状态码 |
| to_grpc_status | `[[nodiscard]] static constexpr grpc_status_t to_grpc_status(error_code_t code) noexcept` | 映射为 gRPC 状态码 |

**使用示例**

```cpp
error_code_t code{error_level_t::error, system_domain_t::database, 1, 2, 0x0010};
status_mapper_t::to_http_status(code);  // → 503 Service Unavailable
status_mapper_t::to_grpc_status(code);  // → DATA_LOSS
code.set_retryable(true);
status_mapper_t::to_http_status(code);  // → 503（retryable 优先）
```

映射决策树详见 [决策树 · 8](../decision_tree.md#8-httpgrpc-状态码映射选择)。
