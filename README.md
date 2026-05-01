# Error System (错误码系统)

这是一个基于 C++17 标准开发的高性能、高可扩展的全局错误码管理系统。它通过位域（Bitfields）将丰富的错误上下文信息封装在一个 64 位的整数中，提供了零开销的错误码构建与解析能力，并内置了基于 JSON 的多语言（i18n）翻译支持。

## 🌟 核心特性 (Core Features)

*   **现代 C++17**: 大量使用 `constexpr`，支持编译期错误码构建，提供极致性能。
*   **零开销位域封装**: 采用 `union` 与位域结合的设计，直接在寄存器层面完成 64 位整数与各错误字段的转换。
*   **丰富的错误上下文**: 统一的 64 位错误码结构，包含了等级（Level）、系统域（Domain）、子系统（Subsystem）、模块（Module）和错误编号（Number），轻松实现问题的快速定位。
*   **多语言支持 (i18n)**: 内置 `json_translator`，支持动态加载 JSON 字典，将冷冰冰的错误码转换为对用户友好的多语言文本。
*   **模块化预定义**: 已经内置了网络 (`network_error`)、数据库 (`database_error`)、中间件 (`middleware_error`) 等丰富的系统域定义，开箱即用。
*   **完备的测试**: 深度集成 GoogleTest，确保核心逻辑坚如磐石。

---

## 🏗️ 错误码结构设计 (Error Code Structure)

本系统将错误码定义为一个 64 位无符号整数 (`uint64_t`)，其内部位域划分如下：

| 位区间 (Bits) | 长度 (Bits) | 字段名 (Field) | 描述 (Description) |
| :--- | :--- | :--- | :--- |
| `63` | 1 | `sign` | 符号位（0=成功，1=错误） |
| `60-62` | 3 | `reserved` | 预留位，供未来扩展使用 |
| `56-59` | 4 | `level` | 错误等级（如 INFO, WARN, ERROR, FATAL） |
| `48-55` | 8 | `system` | 系统域（区分 Database, Network, App 等） |
| `32-47` | 16 | `subsys` | 子系统编号（最大 65535） |
| `16-31` | 16 | `module` | 模块编号（最大 65535） |
| `0-15` | 16 | `number` | 具体错误编号（最大 65535） |

这种设计允许系统在一个返回值中携带完整的路由信息，极大地方便了微服务或大型单体架构中的错误追踪。

---

## 🚀 快速入门 (Quick Start)

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
    error_level_t::FATAL,      // 错误等级
    system_domain_t::DATABASE, // 系统域
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

### 3. 多语言翻译 (i18n)

内置的 `json_translator_t` 支持将错误码映射为具体语言的字符串：

```cpp
#include "error_system/i18n/json_translator.h"

using namespace error_system::i18n;

json_translator_t translator(language_t::ZH_CN);
// translator.load_dict("path/to/dict.json"); // 加载字典

std::string error_msg = translator.translate(db_err);
```

---

## 🛠️ 编译与安装 (Build & Install)

本项目使用 CMake 作为构建系统，并自动处理测试依赖（GoogleTest）。

### 环境要求
*   支持 C++17 的现代编译器 (GCC 7+, Clang 5+, MSVC 19.14+)
*   CMake 3.15 或更高版本

### 编译步骤

```bash
# 1. 克隆项目
# git clone 
# cd error_system

# 2. 生成构建文件
mkdir build && cd build
cmake ..

# 3. 编译
cmake --build .

# 4. 运行单元测试
ctest --output-on-failure
```

---

## 📁 目录结构 (Directory Structure)

```text
error_system/
├── CMakeLists.txt        # CMake 配置
├── include/              # 对外公开的头文件
│   └── error_system/
│       ├── core/         # 核心定义 (error_code_t, error_builder_t 等)
│       ├── domain/       # 系统域定义
│       ├── error_codes/  # 预定义各个领域的具体错误类型
│       ├── i18n/         # 多语言翻译接口
│       └── utils/        # 辅助工具
├── src/                  # 核心实现代码
│   ├── i18n/             # json_translator 的实现
│   └── utils/
└── tests/                # GoogleTest 单元测试
```

## 📄 许可证 (License)

本项目遵循相关开源许可证（详见项目根目录下的 `LICENSE` 文件）。
