# Migration 层 API

`error_system::migration`

错误码废弃与迁移模块，管理错误码的废弃状态与旧码→新码迁移映射，支撑版本演进下的平滑过渡。

---

## deprecation_info_t

废弃信息描述结构。

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| deprecated | `bool` | `false` | 是否已废弃 |
| reason | `std::string` | — | 废弃原因 |
| replacement | `std::optional<error_code_t>` | `nullopt` | 替代错误码（可选） |
| since_version | `std::string` | — | 起始废弃版本 |
| removal_version | `std::string` | — | 计划移除版本 |

---

## deprecation_meta_t

废弃元数据，`mark_deprecated` 的参数封装。与 `deprecation_info_t` 的区别是不含 `deprecated` 布尔字段（标记时隐含为 true），符合函数参数超过 4 个时封装为结构体的设计原则。

| 字段 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| reason | `std::string` | — | 废弃原因 |
| replacement | `std::optional<error_code_t>` | `nullopt` | 替代错误码（可选） |
| since_version | `std::string` | — | 起始废弃版本 |
| removal_version | `std::string` | — | 计划移除版本 |

---

## error_migration_registry_t

错误码废弃与迁移注册器单例，通过 `instance()` 获取。废弃状态与迁移映射分离存储：废弃不一定有替代码，迁移也不一定意味着源码已废弃（可能是别名）。

### 标记

| 方法 | 签名 | 说明 |
|------|------|------|
| mark_deprecated | `void mark_deprecated(error_code_t code, const deprecation_meta_t& meta) noexcept` | 标记废弃（若 meta.replacement 有值，同时建立 migration 映射） |
| register_migration | `void register_migration(error_code_t old_code, error_code_t new_code) noexcept` | 仅建立迁移映射（不标记废弃，适用于别名场景） |

### 查询

| 方法 | 签名 | 说明 |
|------|------|------|
| get_deprecation_info | `[[nodiscard]] std::optional<deprecation_info_t> get_deprecation_info(error_code_t code) const noexcept` | 查询废弃信息 |
| is_deprecated | `[[nodiscard]] bool is_deprecated(error_code_t code) const noexcept` | 检查是否已废弃 |

### 迁移

| 方法 | 签名 | 说明 |
|------|------|------|
| migrate | `[[nodiscard]] std::optional<error_code_t> migrate(error_code_t old_code) const noexcept` | 单跳迁移（a → b，不递归） |
| migrate_recursive | `[[nodiscard]] error_code_t migrate_recursive(error_code_t old_code) const noexcept` | 递归迁移到终点，最大深度 16，环检测后返回当前码 |

### 注销

| 方法 | 签名 | 说明 |
|------|------|------|
| unmark_deprecated | `bool unmark_deprecated(error_code_t code) noexcept` | 移除废弃标记（不清除迁移映射） |
| unregister_migration | `bool unregister_migration(error_code_t old_code) noexcept` | 移除迁移映射 |
| clear_all | `void clear_all() noexcept` | 清除所有废弃标记与迁移映射 |

### 统计

| 方法 | 签名 | 说明 |
|------|------|------|
| deprecated_count | `[[nodiscard]] size_t deprecated_count() const noexcept` | 已废弃错误码数量 |
| migration_count | `[[nodiscard]] size_t migration_count() const noexcept` | 迁移映射数量 |
| get_deprecated_codes | `[[nodiscard]] std::vector<code_t> get_deprecated_codes() const noexcept` | 已废弃错误码列表 |

### 单跳 vs 递归迁移

- `migrate()`：仅一次映射跳转（a → b），适用于单步版本升级
- `migrate_recursive()`：沿链跳转到终点（a → b → c → … → 终点），最大深度 16，环检测安全

决策树详见 [决策树 · 3](../decision_tree.md#3-错误码废弃与迁移决策)。

**使用示例**

```cpp
auto& reg = error_migration_registry_t::instance();
reg.mark_deprecated(ERR_OLD_DB_POOL, {"v2.0 起改用 ERR_DB_POOL_V2", ERR_DB_POOL_V2, "2.0.0", "3.0.0"});
reg.register_migration(ERR_USER_V1, ERR_USER_V2);           // 别名映射
auto migrated = reg.migrate(ERR_OLD_DB_POOL);                // 单跳到 ERR_DB_POOL_V2
```

`unmark_deprecated()` 不会清除迁移映射，便于先停止废弃警告再逐步下线。
