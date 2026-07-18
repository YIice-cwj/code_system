# Changelog

本文件记录 error_system 的所有版本变更。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，版本号遵循 [SemVer](https://semver.org/lang/zh-CN/)。

## [v4.4.1] - 2026-07-18

### Added
- utils 基础设施组件：log.h / bad_alloc_handler.h / singleton.h / tagged_ptr.h / lock_free_queue.h / object_pool.h / sso_string.h
- 可观测性插件：metric_plugin_t 与 log_plugin_t
- 异步 result 链式适配器：std::future<result_t<T>> 支持 then/recover
- 异步通知通道、通知模式（sync / async_queue / sync_deferred）、去重采样器
- result_t Lean 模式（result_t<T, bool Lean> 模板参数化），Release 下基础类型体积 32B→16B（Debug 64B→48B）
- error_context_t::make_minimal 静态工厂，供 Lean 路径零副作用构造
- locale 父链回退机制
- 类型安全：编译期冲突检测 + make_error 防御性断言
- sign 位语义重定义（0=success，非零=failure，对齐 Unix 退出码约定）
- PAYLOAD_SSO_CAPACITY 编译期覆盖宏
- GitHub Actions CI 工作流
- 基准对比文档 docs/benchmark_comparison.md

### Changed
- 通知器架构重构：i_error_notifier_t 收敛为 notify(ctx)+notify(code) 双重载，新增 try_notify 静态封装
- i_error_plugin_t 新增 on_code(code) 默认空实现，支撑 Lean 路径零 error_context 构造
- async_notification_channel 新增 code_queue_ 无锁通道，双线程分别消费 context 与 code
- feature_flags_t 收编 async_queue_max_size / deferred_buffer_max_size / duplicate_policy，统一运行时配置入口
- error_context_t 集成 object_pool 复用 runtime_block_t，消除高频错误路径堆分配
- 全量迁移 fprintf → LOG_ERROR / report_bad_alloc，统一日志格式
- 文档重构为精炼框架（项目简介 / 关键特性 / 兼容依赖 / 快速集成 / 核心示例 / 性能表现）
- 核心模块注释规范化与实现定义分离（.h 声明 / .cc 实现 / .inl 模板实现）
- registry / serializer / result 模块文件重定位至对应子目录
- 文档全量更新至 v4.4.1：sizeof 数据区分 Debug/Release（`result_t<int,false>` 64B/32B、`result_t<int,true>` 48B/16B，因 `#ifndef NDEBUG` 包裹 `checked_` + `created_at_` 共 32B）
- 基准对比文档补充 Lean vs Full 详细对比章节（Release + Debug 双版本，10 次中位数）
- 性能数据基于全量重新编译后重新采集（Release 错误构造 61 ns / 错误传播 63 ns / to_string 140 ns / to_json 144 ns）

### Fixed
- mpsc_queue_t 引入 hazard pointer 消除 use-after-free 风险
- head_/tail_ 添加 alignas(cache_line) 消除生产者/消费者伪共享
- CI 测试不稳定：stacktrace 符号检测 + 异步背压竞态
- error_migration_test 并发测试在 ASan/UBSan 下概率性失败
- async_result_test lambda 参数名 result 与外层变量重名
- CI 编译错误：nonnull-compare 警告 + ASan/UBSan 链接缺失

### Removed
- error_translator 组件（含头文件、源文件、测试文件）
- 废弃的性能基准测试文件
