# Error System (错误码系统)

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-green.svg)](https://cmake.org)
[![GoogleTest](https://img.shields.io/badge/Tests-GoogleTest-yellow.svg)](https://github.com/google/googletest)

这是一个基于 C++17 标准开发的高性能、高可扩展的全局错误码管理系统。它通过位域（Bitfields）将丰富的错误上下文信息封装在一个 64 位的整数中，提供了零开销的错误码构建与解析能力，并内置了基于 JSON 的多语言（i18n）翻译支持。

---

## 目录

- [核心特性](#核心特性-core-features)
- [错误码结构设计](#错误码结构设计-error-code-structure)
- [快速入门](#快速入门-quick-start)
- [架构概览](#架构概览-architecture)
- [工具库](#工具库-utils)
- [编译与安装](#编译与安装-build--install)
- [目录结构](#目录结构-directory-structure)
- [许可证](#许可证-license)

---

## 核心特性 (Core Features)

*   **现代 C++17**: 大量使用 `constexpr`，支持编译期错误码构建，提供极致性能。
*   **零开销位域封装**: 采用 `union` 与位域结合的设计，直接在寄存器层面完成 64 位整数与各错误字段的转换。
*   **丰富的错误上下文**: 统一的 64 位错误码结构，包含了等级（Level）、系统域（Domain）、子系统（Subsystem）、模块（Module）和错误编号（Number），轻松实现问题的快速定位。
*   **多语言支持 (i18n)**: 内置 `json_translator`，支持动态加载 JSON 字典，将冷冰冰的错误码转换为对用户友好的多语言文本。
*   **模块化预定义**: 已经内置了 16 大系统域（Kernel、Network、Database、Middleware、AI、Cloud 等），每个域下包含丰富的子系统和模块枚举，开箱即用。
*   **Traits 类型萃取**: 为所有子系统和模块提供统一的 `traits` 接口，支持编译期枚举与字符串的双向转换。
*   **完备的测试**: 深度集成 GoogleTest，142+ 单元测试确保核心逻辑坚如磐石。

---

## 错误码结构设计 (Error Code Structure)

本系统将错误码定义为一个 64 位无符号整数 (`uint64_t`)，其内部位域划分如下：

| 位区间 (Bits) | 长度 (Bits) | 字段名 (Field) | 描述 (Description) |
| :--- | :--- | :--- | :--- |
| `63` | 1 | `sign` | 符号位（0=成功，1=错误） |
| `60-62` | 3 | `reserved` | 预留位，供未来扩展使用 |
| `56-59` | 4 | `level` | 错误等级（如 DEBUG, INFO, WARN, ERROR, FATAL） |
| `48-55` | 8 | `system` | 系统域（区分 Database, Network, App 等） |
| `32-47` | 16 | `subsys` | 子系统编号（最大 65535） |
| `16-31` | 16 | `module` | 模块编号（最大 65535） |
| `0-15` | 16 | `number` | 具体错误编号（最大 65535） |

这种设计允许系统在一个返回值中携带完整的路由信息，极大地方便了微服务或大型单体架构中的错误追踪。

---

## 快速入门 (Quick Start)

### 1. 构建错误码

系统提供了 `error_builder_t` 作为工厂类来构建错误码：

```cpp
#include "error_system/core/error_builder.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"

using namespace error_system::core;
using namespace error_system::domain;

// 在编译期构建一个数据库模块的严重错误
constexpr auto db_err = error_builder_t::make_error_code(
    error_level_t::fatal,      // 错误等级
    system_domain_t::database, // 系统域
    100,                       // 子系统ID
    200,                       // 模块ID
    404                        // 错误编号
);
```

### 2. 解析错误码

你可以随时解析一个 `error_code_t` 来获取它的各个组成部分，过程是完全零开销的：

```cpp
error_code_t code = db_err;

if (code.get_sign() == 1) { // 判断是否为错误
    auto level  = code.get_level();  // 获取错误等级
    auto system = code.get_system(); // 获取系统域
    auto module = code.get_module(); // 获取模块编号
    auto number = code.get_number(); // 获取具体的错误编号
}
```

### 3. 使用 Traits 进行枚举转换

系统为所有子系统和模块提供了类型萃取，支持编译期枚举与字符串的双向转换：

```cpp
#include "error_system/traits/subsystem/database_subsystem_traits.h"
#include "error_system/subsystem/database_subsystem.h"

using namespace error_system::traits;
using namespace error_system::subsystem;

// 枚举转字符串
constexpr const char* name = subsystem_traits<database_subsystem_t>::to_string(database_subsystem_t::mysql);
// 结果: "mysql"

// 字符串转枚举
constexpr auto subsys = subsystem_traits<database_subsystem_t>::from_string("redis");
// 结果: database_subsystem_t::redis

// 枚举转整数
constexpr auto value = subsystem_traits<database_subsystem_t>::to_int(database_subsystem_t::mysql);
// 结果: 0x0701
```

### 4. 多语言翻译 (i18n)

内置的 `json_translator_t` 支持将错误码映射为具体语言的字符串：

```cpp
#include "error_system/i18n/json_translator.h"

using namespace error_system::i18n;

json_translator_t translator(language_t::zh_cn);
// translator.load_dict("path/to/dict.json"); // 加载字典

std::string error_msg = translator.translate(db_err);
```

---

## 架构概览 (Architecture)

```
┌─────────────────────────────────────────────────────────────┐
│                        Error System                          │
├─────────────────────────────────────────────────────────────┤
│  Core Layer                                                  │
│  ├── error_code_t    (64位错误码数据类)                      │
│  ├── error_builder_t (编译期错误码构建器)                    │
│  ├── error_level_t   (错误等级枚举)                          │
│  └── error_registry_t(错误码注册器)                          │
├─────────────────────────────────────────────────────────────┤
│  Domain Layer                                                │
│  └── system_domain_t (16大系统域定义)                        │
├─────────────────────────────────────────────────────────────┤
│  Subsystem / Module Layer                                    │
│  ├── 16 个子系统枚举 (kernel, network, database, ai...)     │
│  └── 17 个模块枚举   (kernel, network, database..., common) │
├─────────────────────────────────────────────────────────────┤
│  Traits Layer                                                │
│  ├── subsystem_traits<> (枚举↔字符串↔整数 转换)             │
│  └── module_traits<>    (枚举↔字符串↔整数 转换)             │
├─────────────────────────────────────────────────────────────┤
│  i18n Layer                                                  │
│  ├── i_translator_t     (翻译器接口)                         │
│  ├── json_translator_t  (JSON字典翻译实现)                   │
│  └── language_t         (支持语言枚举)                       │
├─────────────────────────────────────────────────────────────┤
│  Utils Layer                                                 │
│  ├── string_utils_t     (字符串处理: hash, trim, format...) │
│  ├── json_utils_t       (JSON解析与字典)                     │
│  └── file_utils_t       (文件读写操作)                       │
└─────────────────────────────────────────────────────────────┘
```

### 支持的系统域

| 系统域 | 枚举值 | 说明 |
|--------|--------|------|
| `system` | 0x00 | 系统基础 |
| `kernel` | 0x01 | 内核/操作系统 |
| `middleware` | 0x02 | 中间件 |
| `application` | 0x03 | 应用程序 |
| `service` | 0x04 | 服务层 |
| `network` | 0x05 | 网络 |
| `storage` | 0x06 | 存储 |
| `database` | 0x07 | 数据库 |
| `security` | 0x08 | 安全 |
| `ai` | 0x09 | 人工智能 |
| `cloud` | 0x0A | 云计算 |
| `edge` | 0x0B | 边缘计算 |
| `iot` | 0x0C | 物联网 |
| `blockchain` | 0x0D | 区块链 |
| `bigdata` | 0x0E | 大数据 |
| `devops` | 0x0F | DevOps |

---

## 工具库 (Utils)

### string_utils_t

提供编译期字符串哈希、格式化、修剪、分割、大小写转换等工具函数：

```cpp
// 编译期 FNV-1a 哈希
constexpr auto h = utils::string_utils_t::hash("hello");

// 字符串格式化
std::string msg = utils::string_utils_t::format("Error: {}", "connection failed");

// 修剪空白
std::string trimmed = utils::string_utils_t::trim("  hello  ");

// 分割字符串
auto parts = utils::string_utils_t::split("a,b,c", ',');
```

### json_utils_t

轻量级 JSON 解析器，支持嵌套键访问：

```cpp
auto dict = utils::json_dict_t::parse(R"({"user": {"name": "Alice"}})");
std::string name = dict->get_value_or("user.name", "Unknown");
```

### file_utils_t

跨平台文件读写工具：

```cpp
auto content = utils::file_utils::read_file("config.json");
utils::file_utils::write_file("output.txt", "Hello, World!");
```

---

## 编译与安装 (Build & Install)

本项目使用 CMake 作为构建系统，并自动处理测试依赖（GoogleTest）。

### 环境要求
*   支持 C++17 的现代编译器 (GCC 7+, Clang 5+, MSVC 19.14+)
*   CMake 3.15 或更高版本

### 编译步骤

```bash
# 1. 克隆项目
git clone <repo-url>
cd error_system

# 2. 生成构建文件
mkdir build && cd build
cmake ..

# 3. 编译
cmake --build . -j$(nproc)

# 4. 运行单元测试
ctest --output-on-failure
```

### 作为子项目使用

```cmake
add_subdirectory(error_system)
target_link_libraries(your_target PRIVATE error_system::error_system)
```

---

## 目录结构 (Directory Structure)

```text
error_system/
├── CMakeLists.txt              # CMake 配置
├── include/error_system/       # 对外公开的头文件
│   ├── core/                   # 核心定义 (error_code_t, error_builder_t, error_level_t)
│   ├── domain/                 # 系统域定义 (system_domain_t)
│   ├── module/                 # 17 个模块枚举定义
│   ├── subsystem/              # 16 个子系统枚举定义
│   ├── traits/                 # 类型萃取
│   │   ├── module/             # 模块 traits (枚举↔字符串↔整数)
│   │   └── subsystem/          # 子系统 traits
│   ├── i18n/                   # 多语言翻译接口与实现
│   │   └── languages/          # JSON 翻译字典 (zh_CN.json, en_US.json)
│   └── utils/                  # 辅助工具 (string, json, file)
├── src/                        # 核心实现代码
│   ├── i18n/                   # json_translator 实现
│   └── utils/                  # 工具函数实现
├── tests/                      # GoogleTest 单元测试
│   ├── traits/                 # traits 测试 (46 个用例)
│   └── utils/                  # 工具库测试 (96 个用例)
├── script/                     # 代码生成脚本
│   ├── generate_i18n.sh        # 生成翻译字典
│   └── generate_traits.sh      # 生成 traits 文件
└── LICENSE                     # 许可证
```

---

## 许可证 (License)

本项目遵循相关开源许可证（详见项目根目录下的 [LICENSE](LICENSE) 文件）。
