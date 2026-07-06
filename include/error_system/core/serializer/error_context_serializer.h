#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "error_system/core/error_context.h"

/**
 * @file error_context_serializer.h
 * @brief 错误上下文序列化器
 * @details 将 error_context_t 转换为人类可读文本、JSON 字符串和紧凑二进制表示。
 *          类禁止实例化，所有方法均为静态方法；唯一可变状态为静态指针
 *          subsystem_module_resolver_（通过 set_subsystem_module_resolver 配置）。
 * @author yiice
 * @version 4.0.0
 * @date 2026-07-06
 * @copyright Copyright (c) 2026
 */
namespace error_system::i18n {
    class i_subsystem_module_resolver_t;
}  // namespace error_system::i18n

namespace error_system::utils::detail {
    class json_lexer_t;
}  // namespace error_system::utils::detail

namespace error_system::core {

    /**
     * @brief 错误上下文序列化器
     * @details 禁止实例化，所有方法为静态方法。通过 error_context_t 的 friend 声明
     *          访问其私有成员 code_、block_、cause_，以及私有方法 ensure_block_()、
     *          insert_or_update_payload_()；并通过 block_ 间接访问 runtime_block_t
     *          的成员（message、metadata、source_location、stack_frames 等）。
     *          文本序列化所需的子系统/模块名称通过 i18n::i_subsystem_module_resolver_t
     *          抽象接口获取，默认绑定到 i18n::subsystem_module_catalog_t 单例，
     *          可由调用方注入自定义实现。
     */
    class error_context_serializer_t {
    public:
        /** 二进制序列化魔数 "ESER"（小端） */
        static constexpr uint32_t BINARY_MAGIC = 0x52455345u;
        /** 二进制序列化格式版本号 */
        static constexpr uint8_t BINARY_VERSION = 1;

    private:
        using json_lexer_t = error_system::utils::detail::json_lexer_t;

        /** JSON 字段解析器函数指针类型：(context, lexer) -> bool */
        using field_parser_t = bool (*)(error_context_t&, json_lexer_t&) noexcept;

        /**
         * @brief 子系统/模块名称解析器指针
         * @details 默认 nullptr，首次文本输出时绑定到 i18n 默认解析器。
         *          裸指针非线程安全，预期在初始化阶段调用 set_subsystem_module_resolver()。
         */
        static const error_system::i18n::i_subsystem_module_resolver_t* subsystem_module_resolver_;

        /** 递归实现 to_string，按深度缩进渲染 cause 链 */
        static std::string to_string_impl_(const error_context_t& context, size_t depth) noexcept;

        /** 递归实现 to_json，按深度渲染 cause 链 */
        static std::string to_json_impl_(const error_context_t& context, size_t depth) noexcept;

        /** 递归实现 to_binary，小端序编码，cause 链追加到流末尾 */
        static std::string to_binary_impl_(const error_context_t& context, size_t depth) noexcept;

        /** 从二进制数据递归反序列化单个 error_context_t 节点（不含魔数/版本头） */
        static std::optional<error_context_t> from_binary_node_(
            std::string_view data, size_t& offset, size_t depth) noexcept;

        /** 解析二进制 location 字段（file/func/line 三子字段全部成功才写入） */
        static bool parse_binary_location_field_(error_context_t& context,
                                                  std::string_view data, size_t& offset) noexcept;

        /** 解析二进制 payload 字段（4 字节计数 + 逐项 key/value） */
        static bool parse_binary_payload_field_(error_context_t& context,
                                                 std::string_view data, size_t& offset) noexcept;

        /** 解析二进制 cause 字段（has_cause 标记 + 递归 from_binary_node_） */
        static bool parse_binary_cause_field_(error_context_t& context,
                                              std::string_view data, size_t& offset, size_t depth) noexcept;

        /** 从 JSON 词法分析器递归反序列化单个 error_context_t 节点（流式解析，不构建中间树） */
        static std::optional<error_context_t> from_json_node_(json_lexer_t& lexer, size_t depth) noexcept;

        /** 解析 JSON "code" 字段（字符串形式错误码 + 注册表补齐元数据） */
        static bool parse_json_code_field_(error_context_t& context, json_lexer_t& lexer) noexcept;

        /** 解析 JSON "message" 字段 */
        static bool parse_json_message_field_(error_context_t& context, json_lexer_t& lexer) noexcept;

        /** 解析 JSON "location" 字段（file/function/line 三子字段全部成功才写入） */
        static bool parse_json_location_field_(error_context_t& context, json_lexer_t& lexer) noexcept;

        /** 解析 JSON "payload" 字段（限制项数 ≤ 100000） */
        static bool parse_json_payload_field_(error_context_t& context, json_lexer_t& lexer) noexcept;

        /** 解析 JSON "stack_frames" 字段（STACKTRACE_ENABLED 关闭时跳过） */
        static bool parse_json_stack_frames_field_(error_context_t& context, json_lexer_t& lexer) noexcept;

        /** 解析 JSON "cause" 字段（递归 from_json_node_） */
        static bool parse_json_cause_field_(error_context_t& context, json_lexer_t& lexer, size_t depth) noexcept;

        /** 解析 payload 对象中的单个键值对（调用前 lexer 已消费 key token） */
        static bool parse_single_payload_entry_(json_lexer_t& lexer, error_context_t& context,
                                                std::string key) noexcept;

        /** JSON 顶层字段分发表（字段名 → 解析器函数指针），静态 const 哈希表，线程安全 */
        static const std::unordered_map<std::string_view, field_parser_t>&
        field_dispatcher_table_() noexcept;

        /** 获取当前解析器，未注入时绑定到 i18n 默认解析器 */
        static const error_system::i18n::i_subsystem_module_resolver_t* get_subsystem_module_resolver_() noexcept;

        /** 构建子系统/模块名称字符串，i18n 关闭时回退为数字形式 */
        static std::string build_subsystem_module_string_(const error_context_t& context) noexcept;

    public:
        error_context_serializer_t() = delete;
        ~error_context_serializer_t() = delete;
        error_context_serializer_t(const error_context_serializer_t&) = delete;
        error_context_serializer_t& operator=(const error_context_serializer_t&) = delete;
        error_context_serializer_t(error_context_serializer_t&&) = delete;
        error_context_serializer_t& operator=(error_context_serializer_t&&) = delete;

        /**
         * @brief 从二进制数据反序列化错误上下文
         * @details 校验魔数与版本号，还原 code/message/location/payload/cause 链。
         *          文件名与函数名由 error_context_t 内部字符串存储持有，保证生命周期安全。
         *          任何格式错误或分配失败均返回 std::nullopt，不抛异常。
         * @param data 二进制数据
         * @return 反序列化成功返回 error_context_t，失败返回 std::nullopt
         */
        [[nodiscard]] static std::optional<error_context_t> from_binary(std::string_view data) noexcept;

        /**
         * @brief 从 JSON 字符串反序列化错误上下文
         * @details 还原 code/message/location/payload/stack_frames/cause 链。
         *          code 字段接受字符串形式（与 to_json 自洽）。
         *          stack_frames 仅在 STACKTRACE_ENABLED 开启时还原。
         *          任何格式错误或分配失败均返回 std::nullopt，不抛异常。
         * @param json JSON 字符串
         * @return 反序列化成功返回 error_context_t，失败返回 std::nullopt
         */
        [[nodiscard]] static std::optional<error_context_t> from_json(std::string_view json) noexcept;

        /**
         * @brief 设置文本序列化使用的子系统/模块名称解析器
         * @param resolver 解析器接口指针，传入 nullptr 恢复默认解析器
         * @note 非线程安全，预期在初始化阶段调用
         */
        static void set_subsystem_module_resolver(
            const error_system::i18n::i_subsystem_module_resolver_t* resolver) noexcept {
            subsystem_module_resolver_ = resolver;
        }

        /**
         * @brief 转换为人类可读字符串
         * @details 输出格式：[Location] [Sign/Level/System/子系统·模块] Code: N (name) - message: description {payload} [Stacktrace] ↳ Caused by: ...
         *          根据 i18n 配置决定子系统/模块输出本地化名称或原始 ID。
         *          cause 链递归渲染，超过 MAX_CAUSE_DEPTH 截断。
         * @param context 错误上下文
         * @return 序列化后的文本
         */
        [[nodiscard]] static std::string to_string(const error_context_t& context) noexcept {
            return to_string_impl_(context, 0);
        }

        /**
         * @brief 转换为 JSON 字符串
         * @details 生成包含 code、message、location、payload、stack_frames、cause 等字段的 JSON
         * @param context 错误上下文
         * @return 序列化后的 JSON 字符串
         */
        [[nodiscard]] static std::string to_json(const error_context_t& context) noexcept {
            return to_json_impl_(context, 0);
        }

        /**
         * @brief 转换为紧凑二进制字符串
         * @details 使用小端序编码，适合高性能 RPC 或持久化存储
         * @param context 错误上下文
         * @return 序列化后的二进制字符串
         */
        [[nodiscard]] static std::string to_binary(const error_context_t& context) noexcept {
            return to_binary_impl_(context, 0);
        }
    };

}  // namespace error_system::core
