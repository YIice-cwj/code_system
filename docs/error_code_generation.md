# 错误码自动生成指南

从 JSON 配置一键生成 C++ 头文件、全局字典与 Markdown 文档。核心思路：JSON 配置是唯一数据源，C++ 头文件、字典注册表、Markdown 文档均由脚本自动生成，编译期检测 ID 冲突。

## 目录结构

```
error_system/
├── config/errors/                    # JSON 配置（数据源）
├── script/
│   ├── generate_errors.sh            # Shell 入口（调用三个 Python 脚本）
│   └── script_py/
│       ├── generate_all.py           # 统一入口
│       ├── generate_error_codes.py   # JSON → C++ 头文件
│       ├── generate_error_dict.py    # 汇总 → error_dict.h + 冲突检测
│       └── generate_error_docs.py    # 汇总 → Markdown 字典
└── build/generated_errors/           # 生成产物（不入版本控制）
    ├── include/<service>_errors.h
    ├── include/error_dict.h
    └── error_dictionary.md
```

## JSON 配置格式

每个服务对应一个 JSON 文件，文件名即为生成的 C++ 头文件名（如 `trade_service_errors.json` → `trade_service_errors.h`）。

```json
{
    "namespace": "biz::trade_errors",
    "service_name": "交易服务",
    "service_i18n": { "zh_CN": "交易服务", "en_US": "Trade Service", "ja_JP": "取引サービス" },
    "default_locale": "zh_CN",
    "domain": "application",
    "subsystem_id": 101,
    "modules": {
        "order": { "id": 1, "desc": "订单模块", "i18n": { "zh_CN": "订单模块", "en_US": "Order Module" } },
        "cart": { "id": 2, "desc": "购物车模块" }
    },
    "errors": [
        { "name": "ERR_ORDER_NOT_FOUND", "module": "order", "level": "error", "number": 1,
          "desc": "订单不存在或已删除", "i18n": { "zh_CN": "订单不存在或已删除", "en_US": "Order not found" } }
    ]
}
```

| 字段 | 类型 | 必填 | 说明 |
|------|------|:---:|------|
| `namespace` | string | 是 | C++ 命名空间（如 `biz::trade_errors`） |
| `service_name` | string | 是 | 服务名称（fallback，用于文档和错误输出） |
| `service_i18n` | object | 否 | 服务名称多语言映射，缺失时用 `service_name` 注册为 `default_locale` |
| `default_locale` | string | 否 | 默认 locale（默认 `zh_CN`），`i18n` 缺失时用 `desc` 注册到此 locale |
| `domain` | string | 是 | 系统域，取值见下表 |
| `subsystem_id` | int | 是 | 子系统 ID（1-65535） |
| `modules` | object | 是 | 模块映射表 |
| `modules.<key>.id` | int | 是 | 模块 ID（1-65535） |
| `modules.<key>.desc` | string | 是 | 模块名称（fallback） |
| `modules.<key>.i18n` | object | 否 | 模块名称多语言映射 |
| `errors` | array | 是 | 错误码列表 |
| `errors[].name` | string | 是 | 错误码宏名（必须以 `ERR_` 开头） |
| `errors[].module` | string | 是 | 模块 key（必须存在于 `modules`） |
| `errors[].level` | string | 是 | 错误级别（debug/info/warn/error/fatal） |
| `errors[].number` | int | 是 | 错误编号（1-65535，模块内唯一） |
| `errors[].desc` | string | 是 | 错误描述（fallback） |
| `errors[].i18n` | object | 否 | 多语言消息映射，缺失时用 `desc` 注册为 `default_locale` |

### i18n 多语言机制

所有 `i18n` 字段均可选，完全向后兼容。`desc`/`service_name` 自动按 `default_locale` 注册为兜底；提供 `i18n` 但未含 `default_locale` 时自动用 `desc` 补齐，保证回退目标始终有值。运行时通过 `i18n_config_t::set_output_locale()` 切换语言。

| 场景 | 配置方式 | 注册行为 |
|------|------|------|
| 中文项目 | 不指定 `i18n` / `default_locale` | `desc` 自动注册为 `zh_CN` |
| 英文项目 | `desc` 为英文，设 `default_locale=en_US` | 注册为 `en_US` |
| 多语言项目 | 显式 `i18n` + `service_i18n` | 按 `i18n` 注册，`desc` 兜底 |

### 系统域取值（`domain`）

| 取值 | 含义 | 取值 | 含义 |
|------|------|------|------|
| `none` | 未分类 | `middleware` | 中间件 |
| `system` | 系统级 | `database` | 数据库 |
| `application` | 应用层 | `third_party` | 第三方依赖 |

### 错误级别取值（`level`）

`debug` < `info` < `warn` < `error` < `fatal`（由低到高）。

## 生成产物

| 产物 | 说明 |
|------|------|
| `<service>_errors.h` | 用 `DEFINE_ERROR_CODE` 宏定义错误码常量，静态初始化在 `main()` 前自动注册；i18n 消息按 locale 独立注册到 `i18n_t`。 |
| `error_dict.h` | 汇总所有 JSON 生成子系统/模块名称注册表，`to_string()` 据此将 ID 转为可读名称。 |
| `error_dictionary.md` | 人类可读错误码字典，每个服务一张表，含宏名、级别、模块、描述。 |

## 使用方式

```bash
# 1. CMake 自动构建（推荐）：配置阶段扫描 config/errors/*.json，构建时自动生成
cmake -S . -B build && cmake --build build

# 2. Shell 脚本
./script/generate_errors.sh

# 3. Python 入口（可指定输出路径）
python script/script_py/generate_all.py [build_dir]
```

`add_custom_command` 配置了 `DEPENDS`，JSON 或 Python 脚本变更时下次构建自动重新生成。也可单独调用 `generate_error_codes.py` / `generate_error_dict.py` / `generate_error_docs.py`。

## CMake 集成

error_system 项目内已自动集成。在你的项目中使用：

```cmake
error_system_generate_codes(
    TARGET my_app
    JSON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config/errors)
```

要求环境已安装 Python3，否则 CMake 配置阶段 `FATAL_ERROR` 终止。生成的头文件会自动添加到 `my_app` 的 include 路径。

## ID 冲突检测

`generate_error_dict.py` 生成全局字典前扫描所有 JSON，检测 `(subsystem_id, module_id, number)` 三元组是否全局唯一。冲突时输出：

```
[错误] 检测到错误码 ID 冲突！
  冲突: subsystem_id=103, module_id=2, number=0x0001
    - payment_service_errors.json: ERR_INSUFFICIENT_BALANCE
    - xxx_errors.json: ERR_XXX
```

冲突时构建失败，避免错误码路由错误进入生产环境。

## 完整示例

新增 `config/errors/inventory_service_errors.json`：

```json
{
    "namespace": "biz::inventory_errors",
    "service_name": "库存服务",
    "domain": "application",
    "subsystem_id": 201,
    "modules": {
        "stock": { "id": 1, "desc": "库存管理" },
        "warehouse": { "id": 2, "desc": "仓库管理" }
    },
    "errors": [
        { "name": "ERR_STOCK_INSUFFICIENT", "module": "stock",     "level": "warn",  "number": 1, "desc": "库存不足" },
        { "name": "ERR_STOCK_NOT_FOUND",    "module": "stock",     "level": "error", "number": 2, "desc": "库存记录不存在" },
        { "name": "ERR_WAREHOUSE_OFFLINE",  "module": "warehouse", "level": "fatal", "number": 1, "desc": "仓库系统离线" }
    ]
}
```

构建后在代码中使用：

```cpp
#include "error_system.h"
#include "inventory_service_errors.h"  // 自动生成

result_t<void> deduct_stock(int product_id, int qty) {
    if (qty > get_available_stock(product_id)) {
        return result_t<void>::make_error(biz::inventory_errors::ERR_STOCK_INSUFFICIENT,
            "商品 {} 库存不足", product_id);
    }
    return result_t<void>::make_success();
}
// main() 前已自动注册，to_string() 输出 "库存服务 / 库存管理"
```

## 常见问题

**Q1：生成的头文件应该提交到版本控制吗？**
不应该。生成产物位于 `build/generated_errors/`（`.gitignore` 已排除），JSON 配置才是数据源，必须提交。

**Q2：Python 没有安装怎么办？**
CMake 检测缺失时跳过生成并输出提示，生成空 `error_dict.h` 占位，编译不报错但无法使用业务错误码，建议安装 Python 3.6+。

**Q3：如何修改已生成的错误码描述？**
只修改 JSON 配置，下次构建自动重新生成，不要手动改 `build/generated_errors/` 下的文件。

**Q4：删除 JSON 文件后对应的头文件会自动删除吗？**
不会。CMake `DEPENDS` 配置后下次构建不再重新生成该头文件，建议手动删除后重新构建。

**Q5：如何为多个微服务统一管理错误码？**
将所有服务的 JSON 配置放在同一 `config/errors/` 目录，每个服务一个文件，生成脚本自动汇总并检测跨服务 ID 冲突。

**Q6：错误码 `number` 字段可以重复吗？**
可以，只要 `(subsystem_id, module_id, number)` 三元组不同即可。

**Q7：能否在 JSON 中使用中文？**
可以，所有 JSON 文件使用 UTF-8 编码，中文字段原样输出到生成的头文件和文档中。
