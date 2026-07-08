# 文档索引

错误系统全量文档导航中心，按用途分类速查。

## API 参考

| 文档 | 内容 |
|------|------|
| [Core 层](api/core.md) | `error_code_t` `error_context_t` `result_t` `error_registry_t` `error_context_serializer_t` `error_builder_t` |
| [i18n 层](api/i18n.md) | `locale_t` `subsystem_module_catalog_t` `i18n_t` 多语言消息回退 |
| [Migration 层](api/migration.md) | `error_migration_registry_t` 废弃标记、单跳/递归迁移 |
| [Mapping 层](api/mapping.md) | `http_status_t` `grpc_status_t` `status_mapper_t` HTTP/gRPC 映射 |
| [Config 层](api/config.md) | `feature_flags_t` `stacktrace_config_t` `formatter_config_t` `i18n_config_t` 三种通知模式 |
| [Plugin 层](api/plugin.md) | 插件接口、注册表、路由分发、日志/指标插件、开发指南 |
| [Bridge 层](api/bridge.md) | C ABI 导出、std::error_code 桥接 |
| [Async 层](api/async.md) | async_result_t 链式异步错误处理 |
| [Utils 层](api/utils.md) | 字符串工具、JSON 解析、文件操作、异步队列、堆栈跟踪 |

## 设计文档

| 文档 | 内容 |
|------|------|
| [架构设计](architecture.md) | 分层架构、模块职责、关键设计决策（24 项）、编译配置 |
| [系统配置指南](configuration_guide.md) | 配置项总览、Lean/Full 选择、场景配置、性能数据、最佳实践 |
| [错误码自动生成](error_code_generation.md) | JSON 配置格式、生成脚本、CMake 集成、自定义错误码 |
| [决策树](decision_tree.md) | 通知模式、查询路径、废弃/迁移、i18n 回退、序列化格式、插件开发、错误传递、HTTP/gRPC 映射 |
| [基准对比](benchmark_comparison.md) | plain int / std::error_code / error_system 构造、传播、序列化对比 |

## 快速导航

- **构建错误码** → [error_code_t](api/core.md#error_code_t)
- **错误上下文** → [error_context_t](api/core.md#error_context_t)
- **Result 错误传递** → [result_t](api/core.md#result_tt)
- **错误码注册** → [error_registry_t](api/core.md#error_registry_t)
- **多语言消息** → [i18n_t](api/i18n.md#i18n_t)
- **废弃/迁移** → [error_migration_registry_t](api/migration.md#error_migration_registry_t)
- **HTTP/gRPC 映射** → [status_mapper_t](api/mapping.md#status_mapper_t)
- **全局配置** → [feature_flags_t](api/config.md#feature_flags_t)
- **配置策略与场景推荐** → [系统配置指南](configuration_guide.md)
- **插件通知** → [plugin_registry_t](api/plugin.md#plugin_registry_t)
- **选型决策** → [决策树](decision_tree.md)
- **基准对比** → [基准对比](benchmark_comparison.md)
- **代码生成工具** → [错误码自动生成](error_code_generation.md)

## 运行测试

```bash
cd build && ctest --output-on-failure
```

| 模块 | 文件数 | 用例数 |
|------|:---:|:---:|
| Core | 13 | 271 |
| Plugin | 8 | 115 |
| Utils | 6 | 140 |
| Config | 2 | 27 |
| Domain | 1 | 12 |
| i18n | 2 | 43 |
| Mapping | 1 | 27 |
| Migration | 1 | 32 |
| Async | 1 | 18 |
| Bridge | 2 | 21 |
| **总计** | **37** | **706** |

另有 `tests/migration/perf/` 5 个基准文件（Google Benchmark v1.8.3）：错误上下文、i18n、插件、Result Lean、plain_error_code。
