#include "error_system/core/serializer/error_context_serializer.h"
#include "error_context_serializer_internal.h"

#include "error_system/config/error_config.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/i18n/i_subsystem_module_resolver.h"
#include "error_system/i18n/subsystem_module_catalog.h"
#include "error_system/utils/bad_alloc_handler.h"
#include "error_system/utils/log.h"

using error_system::config::feature_flags_t;
using error_system::config::formatter_config_t;
using error_system::config::i18n_config_t;
using error_system::core::detail::append_decimal;

/**
 * @file error_context_serializer_text.cc
 * @brief 错误上下文序列化器 - 文本格式实现
 * @details 实现 error_context_serializer_t::to_string 及其递归实现 to_string_impl_。
 *          从 error_context_serializer.cc 拆分而来，仅包含文本格式相关的辅助函数与逻辑。
 *          子系统/模块名称从 i18n_config_t 解析输出 locale，保证与错误码消息使用同一语言。
 * @author yiice
 * @version 3.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    const error_system::i18n::i_subsystem_module_resolver_t*
    error_context_serializer_t::get_subsystem_module_resolver_() noexcept {
        const auto* resolver = config::formatter_config_t::get_subsystem_module_resolver();
        if (resolver != nullptr) {
            return resolver;
        }
        static const auto* default_resolver = error_system::i18n::get_default_subsystem_module_resolver();
        return default_resolver;
    }

    std::string error_context_serializer_t::build_subsystem_module_string_(const error_context_t& context) noexcept {
        std::string subsys_module_str;
        const auto code = context.get_code();
        const bool use_text = i18n_config_t::is_i18n_enabled();
        if (use_text) {
            const auto* resolver = get_subsystem_module_resolver_();
            const auto output_locale = i18n_config_t::resolve_output_locale();
            const auto default_locale = i18n_config_t::get_default_locale();
            try {
                const auto sm_info = resolver->resolve_subsystem_module(
                    output_locale, default_locale, code.get_subsys(), code.get_module());
                subsys_module_str.reserve(sm_info.subsystem_name.size() + sm_info.module_name.size() + 3);
                subsys_module_str.append(sm_info.subsystem_name).append(" / ").append(sm_info.module_name);
            } catch (const std::bad_alloc&) {
                LOG_ERROR("[error_context_serializer] build_subsystem_module_string: text mode failed");
            }
        } else {
            try {
                subsys_module_str.reserve(32);
                subsys_module_str.append("SubSys: ");
                append_decimal(subsys_module_str, code.get_subsys());
                subsys_module_str.append(", Module: ");
                append_decimal(subsys_module_str, code.get_module());
            } catch (const std::bad_alloc&) {
                LOG_ERROR("[error_context_serializer] build_subsystem_module_string: numeric mode failed");
            }
        }
        return subsys_module_str;
    }

    namespace {

        /**
         * @brief 估算文本序列化结果字符串容量，避免多次重分配
         * @param context 错误上下文
         * @param name_size 错误码名称长度
         * @param desc_size 错误码描述长度
         * @param subsys_module_size 子系统/模块字符串长度
         * @return size_t 预估容量
         */
        size_t estimate_string_capacity(const error_context_t& context,
                                        size_t name_size,
                                        size_t desc_size,
                                        size_t subsys_module_size) noexcept {
            const runtime_block_t* blk = context.block();
            const std::string_view msg = blk ? std::string_view(blk->message) : std::string_view{};
            size_t capacity = 96 + name_size + desc_size + subsys_module_size + msg.size();

            context.for_each_payload([&](const std::string& key, const std::string& value) {
                capacity += key.size() + value.size() + 4;
            });

            if constexpr (feature_flags_t::STACKTRACE_ENABLED) {
                const auto frames = context.get_stack_frames();
                if (frames) {
                    for (const auto& frame : *frames) {
                        capacity += frame.size() + 12;
                    }
                }
            }

            if (const error_context_t* cause = context.cause()) {
                const runtime_block_t* cause_blk = cause->block();
                const std::string_view cause_msg = cause_blk ? std::string_view(cause_blk->message) : std::string_view{};
                capacity += 16 + cause_msg.size();
            }
            return capacity;
        }

        /**
         * @brief 追加源位置信息文本（[Location: file:line @ function]）
         * @param result 目标字符串
         * @param context 错误上下文
         */
        void append_location_text(std::string& result, const error_context_t& context) {
            if constexpr (feature_flags_t::LOCATION_ENABLED) {
                const runtime_block_t* blk = context.block();
                if (context.is_location_available()) {
                    result.append(" [Location: ").append(blk->file_name).append(":");
                    append_decimal(result, blk->source_location.line());
                    result.append(" @ ").append(blk->source_location.function_name()).append("]");
                }
            }
        }

        /**
         * @brief 追加签名/等级/系统域/子系统/模块/错误编号子头部
         * @param result 目标字符串
         * @param context 错误上下文
         * @param subsys_module_str 子系统/模块字符串
         */
        void append_subheader_text(std::string& result, const error_context_t& context,
                                   const std::string& subsys_module_str) {
            result.append("[Sign: ")
                .append(context.is_error() ? "Error" : "Success")
                .append(" Level: ")
                .append(core::to_string(context.get_code().get_level()))
                .append(", System: ")
                .append(domain::to_string(context.get_code().get_system()))
                .append(", ")
                .append(subsys_module_str)
                .append("] Code: ");
            append_decimal(result, context.get_code().get_number());
        }

        /**
         * @brief 追加 payload 文本段（{key=value, ...}）
         * @param result 目标字符串
         * @param context 错误上下文
         */
        void append_payload_text(std::string& result, const error_context_t& context) {
            if (context.payload_size() == 0) {
                return;
            }
            result.append(" {");
            bool first = true;
            context.for_each_payload([&](const std::string& key, const std::string& value) {
                if (!first) {
                    result.append(", ");
                }
                result.append(key).append("=").append(value);
                first = false;
            });
            result.push_back('}');
        }

        /**
         * @brief 追加堆栈跟踪文本段（\n  [Stacktrace]:\n    #0  frame...）
         * @param result 目标字符串
         * @param context 错误上下文
         */
        void append_stacktrace_text(std::string& result, const error_context_t& context) {
            if constexpr (feature_flags_t::STACKTRACE_ENABLED) {
                const auto frames = context.get_stack_frames();
                if (!frames || frames->empty()) {
                    return;
                }
                result.append("\n  [Stacktrace]:");
                for (size_t i = 0; i < frames->size(); ++i) {
                    result.append("\n    #");
                    append_decimal(result, i);
                    result.append("  ").append((*frames)[i]);
                }
            }
        }

    }  // namespace

    std::string error_context_serializer_t::to_string_impl_(const error_context_t& context, size_t depth) noexcept {
        if (auto formatter = formatter_config_t::get_custom_formatter()) {
            try {
                return formatter(context);
            } catch (const std::exception& e) {
                LOG_ERROR("[error_context_serializer] to_string: custom formatter threw exception: {}", e.what());
            }
        }

        const auto metadata = context.get_metadata();
        const bool has_metadata = metadata && !metadata->name.empty();
        const std::string_view desc = has_metadata ? std::string_view{metadata->description} : std::string_view{"未注册的未知错误"};
        const std::string_view name = has_metadata ? std::string_view{metadata->name} : std::string_view{"UNKNOWN_ERR_CODE"};
        const std::string subsys_module_str = build_subsystem_module_string_(context);

        const std::string_view msg = context.block_ ? std::string_view(context.block_->message) : std::string_view{};

        std::string result;
        try {
            result.reserve(estimate_string_capacity(context, name.size(), desc.size(), subsys_module_str.size()));
            append_location_text(result, context);
            append_subheader_text(result, context, subsys_module_str);
            result.append(" (").append(name).append(") - ");
            if (!msg.empty()) {
                result.append(msg).append(": ");
            }
            result.append(desc);
            append_payload_text(result, context);
            append_stacktrace_text(result, context);

            if (context.cause_ && depth + 1 < MAX_CAUSE_DEPTH) {
                result.append("\n  ↳ Caused by: ").append(to_string_impl_(*context.cause_, depth + 1));
            } else if (context.cause_) {
                result.append("\n  ↳ ... (cause chain truncated)");
            }
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context_serializer", "to_string");
        }
        return result;
    }

}  // namespace error_system::core
