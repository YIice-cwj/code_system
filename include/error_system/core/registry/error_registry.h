#pragma once
#include <atomic>
#include <cstdint>
#include <iosfwd>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/core/error_metadata.h"
// IWYU pragma: begin_exports
#include "error_system/core/registry/duplicate_policy.h"
#include "error_system/core/error_builder.h"
// IWYU pragma: end_exports
#include "error_system/utils/singleton.h"

#include "error_system/config/feature_flags.h"

/**
 * @file error_registry.h
 * @brief 错误码注册器
 * @details 错误码注册与查询，支持主索引/名称索引/模块索引/子系统索引。
 *          重复处理策略委托给 duplicate_policy_handler_t。
 *          子系统/模块名称映射请使用 i18n::subsystem_module_catalog_t。
 * @author yiice
 * @version 3.1.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 错误码注册器
     * @details 单例模式（std::call_once + std::once_flag）。
     *          支持 register/unregister/find 四类索引查询。
     *          线程本地缓存（thread_local 环形缓冲）用于热路径优化。
     */
    class error_registry_t : public utils::singleton_t<error_registry_t> {
        friend class utils::singleton_t<error_registry_t>;
    private:
        std::unordered_map<code_t, error_metadata_t> primary_index_;
        std::unordered_map<std::string, code_t> name_index_;
        std::unordered_map<module_group_id_t, std::vector<code_t>> module_index_;
        std::unordered_map<uint16_t, std::unordered_set<module_group_id_t>> subsystem_index_;
        mutable std::shared_mutex index_mutex_;
        duplicate_policy_handler_t duplicate_handler_{};

        /**
         * @brief 注册表变更纪元（epoch）
         * @details 任何注册/注销变更均 fetch_add+1（release 序），用于驱动线程本地
         *          元数据缓存失效检测。读取使用 acquire 序，与 release 配对。
         */
        static std::atomic<uint64_t> epoch_counter_;

        /**
         * @brief 注册表变更后调用，bump 纪元使所有线程本地缓存失效
         */
        void bump_epoch_() noexcept {
            epoch_counter_.fetch_add(1, std::memory_order_release);
        }

        /**
         * @brief 为批量注册提前预留索引容量
         * @details reserve 失败不影响正确性，仅影响性能。
         */
        void reserve_for_registration_(size_t additional_entries) noexcept;

        /**
         * @brief 按模块组预分配 module_index_ 桶容量
         */
        void preallocate_module_buckets_(const std::vector<error_code_t>& codes) noexcept;

        /**
         * @brief 注册单个错误码到所有索引（已持有锁）
         * @details 处理重复策略、移除旧条目、插入新条目到所有索引。
         *          warn 策略下需调用用户回调，回调可能读注册表（再获取 index_mutex_ 共享锁），
         *          持有写锁时调用会自死锁。先快照元数据，临时释放 lock 调用 apply_duplicate_policy，
         *          再重新加锁。skip/overwrite 不触发回调，无需释放锁。
         * @return true=注册成功，false=跳过（重复策略拒绝或分配失败）
         */
        bool register_single_entry_(std::unique_lock<std::shared_mutex>& lock,
                                    error_code_t code, std::string_view name,
                                    std::string_view description) noexcept;

        /**
         * @brief 从模块索引中移除指定错误码
         */
        void erase_from_module_index_(module_group_id_t module_group_id, code_t identity_code) noexcept;

        /**
         * @brief 从子系统索引中移除空的模块组条目
         */
        void erase_from_subsystem_index_(uint16_t subsystem_id, module_group_id_t module_group_id) noexcept;

        /**
         * @brief 默认构造函数（单例模式私有构造）
         */
        error_registry_t() noexcept = default;

    public:
        /**
         * @brief 注册错误码
         */
        void register_error(const error_code_t code,
                            const std::string_view name,
                            const std::string_view description) noexcept;

        /**
         * @brief 批量注册错误码
         * @return 实际注册成功的错误码数量
         * @note 如果数组长度不一致，返回0且不执行任何注册
         */
        [[nodiscard]] size_t register_errors(const std::vector<error_code_t>& codes,
                               const std::vector<std::string_view>& names,
                               const std::vector<std::string_view>& descriptions) noexcept;

        /**
         * @brief 注销错误码（按码值）
         * @details 若不存在则静默忽略。
         */
        void unregister_error(const error_code_t code) noexcept;

        /**
         * @brief 注销错误码（按名称）
         * @details 若不存在则静默忽略。
         */
        void unregister_error(const std::string_view name) noexcept;

        /**
         * @brief 注销模块组的所有错误码
         * @details 同步清除所有索引中该模块的条目，O(模块内错误数)。
         */
        void unregister_module(const module_group_id_t module_group_id) noexcept;

        /**
         * @brief 注销所有错误码
         */
        void unregister_all() noexcept;

        /**
         * @brief 设置重复处理策略
         * @details 转发至 config::feature_flags_t（全局配置）并同步至 duplicate_handler_
         */
        void set_duplicate_policy(duplicate_policy_t policy) noexcept {
            config::feature_flags_t::set_duplicate_policy(policy);
            duplicate_handler_.set_policy(policy);
        }

        /**
         * @brief 获取当前重复处理策略
         * @return 当前重复处理策略
         */
        duplicate_policy_t get_duplicate_policy() const noexcept {
            return config::feature_flags_t::get_duplicate_policy();
        }

        /**
         * @brief 设置重复注册警告回调
         * @note 传入 nullptr 可清除回调；转发至 duplicate_handler_
         */
        void set_duplicate_warn_callback(duplicate_warn_callback_t callback) noexcept {
            duplicate_handler_.set_warn_callback(std::move(callback));
        }

        /**
         * @brief 获取当前重复注册警告回调
         * @return 当前回调的拷贝（线程安全）
         */
        duplicate_warn_callback_t get_duplicate_warn_callback() const noexcept {
            return duplicate_handler_.get_warn_callback();
        }

        /**
         * @brief 检查错误码是否已注册
         * @return 已注册返回 true，否则 false
         */
        [[nodiscard]] bool is_registered(const error_code_t code) const noexcept;

        /**
         * @brief 通过 64位错误码获取详情（值副本，线程安全）
         * @details 返回值副本而非指针，避免锁释放后被另一线程注销导致 use-after-free。
         */
        [[nodiscard]] std::optional<error_metadata_t> get_info(const error_code_t code) const noexcept;

        /**
         * @brief 通过模块 ID 获取所有错误码（值副本，线程安全）
         */
        [[nodiscard]] std::vector<error_metadata_t>
        get_errors_by_module(const module_group_id_t module_group_id) const noexcept;

        /**
         * @brief 通过子系统 ID 获取该子系统下所有错误码（值副本，线程安全）
         */
        [[nodiscard]] std::vector<error_metadata_t>
        get_errors_by_subsystem(uint16_t subsystem_id) const noexcept;

        /**
         * @brief 通过错误码名称查找错误码
         * @param name 错误码宏名称
         * @return 找到返回错误码，未找到返回 std::nullopt
         */
        [[nodiscard]] std::optional<error_code_t> find_by_name(const std::string_view name) const noexcept;

        /**
         * @brief 获取当前注册表纪元（用于缓存失效检测）
         * @return 当前纪元计数值
         */
        [[nodiscard]] uint64_t get_epoch() const noexcept {
            return epoch_counter_.load(std::memory_order_acquire);
        }

        /**
         * @brief 线程本地缓存的元数据查询（热路径优化）
         * @details 使用 thread_local 环形缓存（容量 16），缓存命中时零锁开销。
         *          注册表任何变更（register/unregister）会 bump 纪元，
         *          缓存检测到纪元变化时整体失效重建。
         *          缓存同时记录"未注册"结果（nullopt），避免对未注册码重复查询。
         */
        [[nodiscard]] std::optional<error_metadata_t> get_info_cached(const error_code_t code) const noexcept;

        /**
         * @brief 清除当前线程的元数据缓存
         * @details 仅用于测试与显式刷新场景。正常运行时无需调用。
         */
        void invalidate_metadata_cache() const noexcept;
    };

    /**
     * @brief 错误码自动注册辅助类
     * @details 配合 DEFINE_ERROR_CODE 宏使用，利用静态初始化在 main 函数前注册错误码。
     *          子系统/模块名称需通过 i18n::subsystem_module_catalog_t::instance() 单独注册。
     */
    struct error_registrar_t {
        /**
         * @brief 构造函数（自动注册错误码）
         * @param subsystem_name 子系统名称（已废弃，请通过 i18n::subsystem_module_catalog_t 注册）
         * @param module_name 模块名称（已废弃，请通过 i18n::subsystem_module_catalog_t 注册）
         */
        error_registrar_t(const error_code_t code,
                          const char* name,
                          const char* description,
                          [[maybe_unused]] const char* subsystem_name = "未知子系统",
                          [[maybe_unused]] const char* module_name = "未知模块") noexcept {
            error_registry_t::instance().register_error(code, name, description);
        }

        error_registrar_t(const error_registrar_t&) = delete;
        error_registrar_t& operator=(const error_registrar_t&) = delete;
        error_registrar_t(error_registrar_t&&) = delete;
        error_registrar_t& operator=(error_registrar_t&&) = delete;
    };

}  // namespace error_system::core

/**
 * @brief 定义并自动注册错误码的宏
 * @param NAME 错误码名称（宏名）
 * @param LEVEL 错误等级 (error_level_t)
 * @param SYSTEM 系统域 (system_domain_t)
 * @param SUBSYS 子系统 ID
 * @param MODULE 模块 ID
 * @param NUMBER 错误编号
 * @param DESC 错误描述字符串
 * @param SUBSYS_NAME 子系统名称（已废弃，保留参数以向后兼容）
 * @param MODULE_NAME 模块名称（已废弃，保留参数以向后兼容）
 * @details 该宏会：
 *          1. 创建一个 constexpr error_code_t 常量（编译期可用，无初始化顺序问题）
 *          2. 在动态初始化阶段自动将错误码注册到 error_registry_t
 * @note 子系统/模块名称不再由本宏自动注册，需调用方通过
 *       i18n::subsystem_module_catalog_t::instance().register_subsystem_module(...) 单独注册
 * @note 必须在全局命名空间使用
 * @note 静态初始化顺序安全性：
 *       - error_registry_t 单例使用 std::call_once + 函数局部静态，跨 TU 安全；
 *       - registrar 使用 C++17 inline 变量，全程序仅初始化一次；
 *       - constexpr error_code_t 常量为编译期决议，无 SIOF 风险；
 *       - 但请勿在其它 TU 的静态初始化代码中查询注册表（如
 *         static auto meta = registry.get_info(MY_CODE)），因为跨 TU 动态
 *         初始化顺序未指定。运行时查询（如 error_context_t 构造）不受影响，
 *         因其发生在 main() 之后所有静态初始化完成时。
 * @example
 * DEFINE_ERROR_CODE(
 *     ERR_DB_CONNECTION_TIMEOUT,
 *     error_system::core::error_level_t::error,
 *     error_system::domain::system_domain_t::database,
 *     1, 1, 0x0001,
 *     "数据库连接超时",
 *     "数据库服务",
 *     "连接管理")
 */
#define DEFINE_ERROR_CODE(NAME, LEVEL, SYSTEM, SUBSYS, MODULE, NUMBER, DESC, SUBSYS_NAME, MODULE_NAME)                  \
    constexpr ::error_system::core::error_code_t NAME(LEVEL, SYSTEM, SUBSYS, MODULE, NUMBER);                           \
    inline const ::error_system::core::error_registrar_t NAME##_registrar_(NAME, #NAME, DESC, SUBSYS_NAME, MODULE_NAME);
