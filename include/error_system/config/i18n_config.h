#pragma once
#include <atomic>
#include <cstdint>
#include <mutex>
#include <optional>

#include "error_system/i18n/locale.h"

/**
 * @file i18n_config.h
 * @brief i18n 配置类
 * @details 从 error_config_t 拆分而来，单一职责：管理 i18n（多语言）功能的启用开关
 *          与输出语言区域配置。线程安全（使用 std::atomic 无锁读写）。
 *
 *          配置项：
 *          - enable_i18n：总开关，false 时序列化输出回退为原始 ID 数字
 *          - output_locale：输出语言区域（显式设置后使用此值）
 *          - default_locale：默认语言区域（output_locale 未设置时回退使用）
 *
 *          locale 解析顺序：output_locale（若已设置）→ default_locale
 *
 *          本类仅持有配置状态，不直接调用 i18n_t / error_registry_t。
 *          序列化器从本类读取 locale 后，显式传给 registry / i18n_t 查询本地化文本。
 * @author yiice
 * @version 3.0.0
 * @date 2026-06-29
 * @copyright Copyright (c) 2026
 */
namespace error_system::config {

    using error_system::i18n::locale_t;

    /**
     * @brief i18n 配置类
     * @details 管理 i18n 启用开关与输出 locale 配置。类仅包含静态成员，禁止实例化。
     *
     * @note 与 i18n_t 的关系：
     *       本类是 locale 配置的唯一入口。i18n_t 的 set_default_locale / set_active_locale 等
     *       接口保留以兼容现有调用方，但内部已委托给本类，避免双源配置不同步。
     *       序列化器应从本类读取 locale 配置，保证 config 层单一职责。
     */
    class i18n_config_t {
    private:
        /**
         * @brief output locale 已设置标志位（位于 get_output_locale_storage_ 的 bit 8）
         * @details 与 locale 值合并存储于同一 uint16_t 原子变量，保证读写的原子一致性
         */
        static constexpr uint16_t OUTPUT_LOCALE_SET_FLAG = 0x100;

        /**
         * @brief i18n 启用标志位存储
         * @details 使用 std::atomic<bool> 保证无锁并发读写
         * @return std::atomic<bool>& i18n 启用标志位引用
         */
        static std::atomic<bool>& get_flag_i18n_enabled_() noexcept;

        /**
         * @brief 默认 locale 存储（使用 uint8_t 存储 locale_t 的 underlying value）
         * @details 直接存储枚举的底层值，避免对 std::atomic<locale_t> 锁自由性的平台依赖
         * @return std::atomic<uint8_t>& 默认 locale 存储引用
         */
        static std::atomic<uint8_t>& get_default_locale_storage_() noexcept;

        /**
         * @brief 输出 locale 存储（值与已设置标志合并为单一原子变量）
         * @details 显式设置的输出 locale，未设置时回退到 default_locale。
         *          使用 uint16_t 存储：低 8 位为 locale 值，bit 8 (OUTPUT_LOCALE_SET_FLAG)
         *          为已设置标志。值与标志合并为单一原子变量，避免分开存储时的读写竞争。
         * @return std::atomic<uint16_t>& 输出 locale 存储引用
         */
        static std::atomic<uint16_t>& get_output_locale_storage_() noexcept;

        /**
         * @brief locale parent 覆盖存储（运行时可配置的 parent 链）
         * @details 使用 LOCALE_COUNT 个 std::atomic<uint8_t> 数组，每个 locale 对应一个槽位。
         *          首次访问时通过 std::call_once 用 LOCALE_PARENT_TABLE 初始化默认值，
         *          之后调用方可通过 set_locale_parent() 覆盖个别 locale 的 parent。
         * @return std::atomic<uint8_t>(&)[LOCALE_COUNT] parent 存储数组引用
         */
        static std::atomic<uint8_t>* get_locale_parent_storage_() noexcept;

    public:
        i18n_config_t() = delete;

        ~i18n_config_t() = delete;

        i18n_config_t(const i18n_config_t&) = delete;

        i18n_config_t& operator=(const i18n_config_t&) = delete;

        i18n_config_t(i18n_config_t&&) = delete;

        i18n_config_t& operator=(i18n_config_t&&) = delete;

        /**
         * @brief 启用/禁用 i18n 功能
         * @details false 时序列化输出回退为原始 ID 数字，
         *          true 时按 output_locale / default_locale 查询本地化文本。
         * @param enable 是否启用
         */
        static void set_enable_i18n(bool enable) noexcept;

        /**
         * @brief 检查 i18n 功能是否启用
         * @return bool 是否启用
         */
        [[nodiscard]] static bool is_i18n_enabled() noexcept;

        /**
         * @brief 设置默认 locale（回退查询使用）
         * @param locale 默认语言区域
         */
        static void set_default_locale(locale_t locale) noexcept;

        /**
         * @brief 获取默认 locale
         * @return locale_t 默认语言区域
         */
        [[nodiscard]] static locale_t get_default_locale() noexcept;

        /**
         * @brief 设置输出 locale（运行时切换语言）
         * @details 设置后序列化器使用此 locale 查询本地化文本。
         *          locale 值与已设置标志合并为单次原子写，保证读端观察到一致状态。
         * @param locale 输出语言区域
         */
        static void set_output_locale(locale_t locale) noexcept;

        /**
         * @brief 清除输出 locale，回退到默认 locale
         */
        static void clear_output_locale() noexcept;

        /**
         * @brief 获取显式设置的输出 locale
         * @return std::optional<locale_t> 已设置则返回 locale，未设置返回 nullopt
         */
        [[nodiscard]] static std::optional<locale_t> get_output_locale() noexcept;

        /**
         * @brief 解析最终输出 locale
         * @details 解析顺序：output_locale（若已设置）→ default_locale
         * @return locale_t 最终输出语言区域
         */
        [[nodiscard]] static locale_t resolve_output_locale() noexcept;

        /**
         * @brief 覆盖指定 locale 的 parent（运行时自定义回退链）
         * @details 覆盖 i18n::parent_locale() 的内置默认值。例如可将 fr_CA（若新增）的 parent
         *          设为 fr_FR，或将 zh_TW 的 parent 改为 en_US 跳过 zh_CN。
         * @note child == parent 时表示该 locale 为链终点；child == en_US 时不允许覆盖（始终为链终点）
         * @param child 子 locale
         * @param parent 父 locale
         */
        static void set_locale_parent(locale_t child, locale_t parent) noexcept;

        /**
         * @brief 查询指定 locale 的当前 parent（含运行时覆盖）
         * @param child 子 locale
         * @return locale_t parent locale；非法值或 en_US 返回 en_US
         */
        [[nodiscard]] static locale_t get_locale_parent(locale_t child) noexcept;

        /**
         * @brief 重置指定 locale 的 parent 为内置默认值
         * @param child 子 locale
         */
        static void reset_locale_parent(locale_t child) noexcept;

        /**
         * @brief 重置所有 locale 的 parent 为内置默认值
         */
        static void reset_all_locale_parents() noexcept;
    };

    inline std::atomic<bool>& i18n_config_t::get_flag_i18n_enabled_() noexcept {
        static std::atomic<bool> enabled{true};
        return enabled;
    }

    inline std::atomic<uint8_t>& i18n_config_t::get_default_locale_storage_() noexcept {
        static std::atomic<uint8_t> locale{static_cast<uint8_t>(locale_t::zh_CN)};
        return locale;
    }

    inline std::atomic<uint16_t>& i18n_config_t::get_output_locale_storage_() noexcept {
        static std::atomic<uint16_t> locale{0};
        return locale;
    }

    inline void i18n_config_t::set_enable_i18n(bool enable) noexcept {
        get_flag_i18n_enabled_().store(enable);
    }

    inline bool i18n_config_t::is_i18n_enabled() noexcept {
        return get_flag_i18n_enabled_().load();
    }

    inline void i18n_config_t::set_default_locale(locale_t locale) noexcept {
        get_default_locale_storage_().store(static_cast<uint8_t>(locale));
    }

    inline locale_t i18n_config_t::get_default_locale() noexcept {
        return static_cast<locale_t>(get_default_locale_storage_().load());
    }

    inline void i18n_config_t::set_output_locale(locale_t locale) noexcept {
        const uint16_t value = static_cast<uint16_t>(locale) | OUTPUT_LOCALE_SET_FLAG;
        get_output_locale_storage_().store(value);
    }

    inline void i18n_config_t::clear_output_locale() noexcept {
        get_output_locale_storage_().store(0);
    }

    inline std::optional<locale_t> i18n_config_t::get_output_locale() noexcept {
        const uint16_t value = get_output_locale_storage_().load();
        if ((value & OUTPUT_LOCALE_SET_FLAG) != 0) {
            return static_cast<locale_t>(static_cast<uint8_t>(value));
        }
        return std::nullopt;
    }

    inline locale_t i18n_config_t::resolve_output_locale() noexcept {
        const uint16_t value = get_output_locale_storage_().load();
        if ((value & OUTPUT_LOCALE_SET_FLAG) != 0) {
            return static_cast<locale_t>(static_cast<uint8_t>(value));
        }
        return static_cast<locale_t>(get_default_locale_storage_().load());
    }

    inline void i18n_config_t::set_locale_parent(locale_t child, locale_t parent) noexcept {
        const auto idx = static_cast<size_t>(child);
        if (idx >= error_system::i18n::LOCALE_COUNT) {
            return;
        }
        if (child == locale_t::en_US) {
            return;
        }
        get_locale_parent_storage_()[idx].store(static_cast<uint8_t>(parent));
    }

    inline locale_t i18n_config_t::get_locale_parent(locale_t child) noexcept {
        const auto idx = static_cast<size_t>(child);
        if (idx >= error_system::i18n::LOCALE_COUNT) {
            return locale_t::en_US;
        }
        return static_cast<locale_t>(get_locale_parent_storage_()[idx].load());
    }

    inline void i18n_config_t::reset_locale_parent(locale_t child) noexcept {
        const auto idx = static_cast<size_t>(child);
        if (idx >= error_system::i18n::LOCALE_COUNT) {
            return;
        }
        get_locale_parent_storage_()[idx].store(
            static_cast<uint8_t>(error_system::i18n::LOCALE_PARENT_TABLE[idx]));
    }

}  // namespace error_system::config
