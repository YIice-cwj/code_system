#include "error_system/core/serializer/error_context_serializer.h"
#include "error_context_serializer_internal.h"

#include <optional>

#include "error_system/config/error_config.h"
#include "error_system/core/registry/error_registry.h"
#include "error_system/utils/bad_alloc_handler.h"
#include "error_system/utils/log.h"

using error_system::config::feature_flags_t;

/**
 * @file error_context_serializer_binary.cc
 * @brief 错误上下文序列化器 - 二进制格式实现
 * @details 实现 error_context_serializer_t 的二进制序列化（to_binary / to_binary_impl_）
 *          与反序列化（from_binary / from_binary_node_）。
 *          从 error_context_serializer.cc 拆分而来，仅包含二进制格式相关的辅助函数与逻辑。
 *          使用小端序编码，顶层包含魔数与版本号；cause 链通过递归追加。
 * @author yiice
 * @version 3.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {
    namespace {

        /**
         * @brief 以小端序写入整数到缓冲区
         * @tparam T 整数类型
         */
        template <typename T>
        void write_little_endian(std::string& buffer, T value) noexcept {
            static_assert(std::is_integral_v<T>, "T must be an integral type");
            for (size_t i = 0; i < sizeof(T); ++i) {
                buffer.push_back(static_cast<char>((value >> (i * 8)) & 0xFF));
            }
        }

        /**
         * @brief 写入长度前缀字符串（4 字节小端长度 + 字符串字节）
         * @param buffer 目标缓冲区
         * @param text 待写入的字符串
         */
        void write_string_len_prefixed(std::string& buffer, std::string_view text) noexcept {
            const size_t string_size = text.size();
            if (string_size > 0xFFFFFFFFULL) {
                LOG_WARN("[error_context_serializer] write_string_len_prefixed: string too long, truncated");
            }
            const uint32_t length = static_cast<uint32_t>(string_size > 0xFFFFFFFFULL ? 0xFFFFFFFFULL : string_size);
            write_little_endian(buffer, length);
            try {
                buffer.append(text.data(), length);
            } catch (const std::bad_alloc&) {
                LOG_ERROR("[error_context_serializer] to_binary: write_string append failed");
            }
        }

        /**
         * @brief 写入源位置（has_location 标记 + file/func/line）
         * @param buffer 目标缓冲区
         * @param context 错误上下文
         */
        void write_location_binary(std::string& buffer, const error_context_t& context) noexcept {
            uint8_t has_location = 0;
            const runtime_block_t* blk = nullptr;
            if constexpr (feature_flags_t::LOCATION_ENABLED) {
                blk = context.block();
                if (context.is_location_available()) {
                    has_location = 1;
                }
            }
            buffer.push_back(static_cast<char>(has_location));
            if (has_location && blk != nullptr) {
                write_string_len_prefixed(buffer, blk->file_name);
                write_string_len_prefixed(buffer, blk->source_location.function_name());
                write_little_endian(buffer, blk->source_location.line());
            }
        }

        /**
         * @brief 写入 payload（4 字节计数 + 各 key/value 长度前缀字符串）
         * @param buffer 目标缓冲区
         * @param context 错误上下文
         */
        void write_payload_binary(std::string& buffer, const error_context_t& context) noexcept {
            const size_t total = context.payload_size();
            if (total > 0xFFFFFFFFULL) {
                LOG_WARN("[error_context_serializer] write_payload_binary: payload count overflow, truncated");
            }
            write_little_endian(buffer, static_cast<uint32_t>(total > 0xFFFFFFFFULL ? 0xFFFFFFFFULL : total));
            context.for_each_payload([&](const std::string& key, const std::string& value) {
                write_string_len_prefixed(buffer, key);
                write_string_len_prefixed(buffer, value);
            });
        }

        /**
         * @brief 从数据视图读取小端序整数
         * @details 读取失败（数据不足）时返回 false 且不修改 offset
         */
        template <typename T>
        bool read_little_endian(std::string_view data, size_t& offset, T& output) noexcept {
            static_assert(std::is_integral_v<T>, "T must be an integral type");
            if (offset + sizeof(T) > data.size()) {
                return false;
            }
            T value = 0;
            for (size_t i = 0; i < sizeof(T); ++i) {
                value |= static_cast<T>(static_cast<uint8_t>(data[offset + i])) << (i * 8);
            }
            offset += sizeof(T);
            output = value;
            return true;
        }

        /**
         * @brief 读取单字节标记
         * @param data 数据视图
         * @param offset 当前偏移量（读取成功时递增）
         * @param output 输出参数，读取的字节值
         * @return bool true=成功，false=数据不足
         */
        bool read_byte(std::string_view data, size_t& offset, uint8_t& output) noexcept {
            if (offset >= data.size()) {
                return false;
            }
            output = static_cast<uint8_t>(data[offset]);
            ++offset;
            return true;
        }

        /**
         * @brief 读取长度前缀字符串（4 字节小端长度 + 字符串字节）
         * @param data 数据视图
         * @param offset 当前偏移量（读取成功时递增）
         * @param output 输出参数，读取的字符串
         * @return bool true=成功，false=数据不足或长度超限
         */
        bool read_string_len_prefixed(std::string_view data, size_t& offset, std::string& output) noexcept {
            uint32_t length = 0;
            if (!read_little_endian(data, offset, length)) {
                return false;
            }
            if (length > MAX_STRING_LENGTH) {
                LOG_WARN("[error_context_serializer] read_string_len_prefixed: length {} exceeds max {}",
                         length, MAX_STRING_LENGTH);
                return false;
            }
            if (offset + length > data.size()) {
                return false;
            }
            try {
                output.assign(data.data() + offset, length);
            } catch (const std::bad_alloc&) {
                utils::report_bad_alloc("error_context_serializer", "read_string_len_prefixed");
                return false;
            }
            offset += length;
            return true;
        }

    }  // namespace

    std::string error_context_serializer_t::to_binary_impl_(const error_context_t& context, size_t depth) noexcept {
        std::string buf;
        const size_t total_payload = context.payload_size();
        const std::string_view msg = context.block_ ? std::string_view(context.block_->message) : std::string_view{};
        try {
            buf.reserve(128 + msg.size() + total_payload * 24);
            if (depth == 0) {
                write_little_endian(buf, BINARY_MAGIC);
                buf.push_back(static_cast<char>(BINARY_VERSION));
            }
            write_little_endian(buf, context.code_.get_code());
            write_string_len_prefixed(buf, msg);
            write_location_binary(buf, context);
            write_payload_binary(buf, context);

            if (context.cause_ && depth + 1 < MAX_CAUSE_DEPTH) {
                buf.push_back(1);
                std::string cause_binary = to_binary_impl_(*context.cause_, depth + 1);
                write_string_len_prefixed(buf, cause_binary);
            } else {
                buf.push_back(0);
            }
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("error_context_serializer", "to_binary");
        }
        return buf;
    }

    /**
     * @brief 从二进制数据反序列化错误上下文（顶层入口）
     * @details 校验魔数与版本号，调用 from_binary_node_ 解析节点，
     *          并拒绝顶层尾随数据。任何校验失败返回 std::nullopt。
     */
    std::optional<error_context_t> error_context_serializer_t::from_binary(std::string_view data) noexcept {
        size_t offset = 0;
        uint32_t magic = 0;
        if (!read_little_endian(data, offset, magic)) {
            return std::nullopt;
        }
        if (magic != BINARY_MAGIC) {
            return std::nullopt;
        }
        uint8_t version = 0;
        if (!read_byte(data, offset, version)) {
            return std::nullopt;
        }
        if (version != BINARY_VERSION) {
            return std::nullopt;
        }
        auto result = from_binary_node_(data, offset, 0);
        if (!result) {
            return std::nullopt;
        }
        if (offset != data.size()) {
            return std::nullopt;
        }
        return result;
    }

    /**
     * @brief 解析二进制 location 字段（file/func/line 三子字段全部成功才写入）
     * @param context 目标上下文
     * @param data 数据视图
     * @param offset 当前偏移量（解析成功时递增）
     * @return bool true=成功，false=格式错误或数据不足
     */
    bool error_context_serializer_t::parse_binary_location_field_(
        error_context_t& context, std::string_view data, size_t& offset) noexcept {
        uint8_t has_location = 0;
        if (!read_byte(data, offset, has_location)) {
            return false;
        }
        if (has_location == 0) {
            return true;
        }
        std::string file;
        std::string func;
        uint32_t line = 0;
        if (!read_string_len_prefixed(data, offset, file) ||
            !read_string_len_prefixed(data, offset, func) ||
            !read_little_endian(data, offset, line)) {
            return false;
        }
        context.ensure_block_();
        if (!context.block_) {
            return false;
        }
        context.block_->loc_file_storage = std::move(file);
        context.block_->loc_func_storage = std::move(func);
        context.block_->file_name = context.block_->loc_file_storage.c_str();
        context.block_->source_location = utils::source_location_t(
            context.block_->loc_file_storage.c_str(), context.block_->loc_func_storage.c_str(), line);
        return true;
    }

    /**
     * @brief 解析二进制 payload 字段（4 字节计数 + 各 key/value 长度前缀字符串）
     * @param context 目标上下文
     * @param data 数据视图
     * @param offset 当前偏移量（解析成功时递增）
     * @return bool true=成功，false=格式错误或超出限制
     */
    bool error_context_serializer_t::parse_binary_payload_field_(
        error_context_t& context, std::string_view data, size_t& offset) noexcept {
        uint32_t payload_count = 0;
        if (!read_little_endian(data, offset, payload_count)) {
            return false;
        }
        if (payload_count > MAX_PAYLOAD_ITEMS) {
            return false;
        }
        for (uint32_t i = 0; i < payload_count; ++i) {
            std::string key;
            std::string value;
            if (!read_string_len_prefixed(data, offset, key) ||
                !read_string_len_prefixed(data, offset, value)) {
                return false;
            }
            context.insert_or_update_payload_(std::move(key), std::move(value));
        }
        return true;
    }

    /**
     * @brief 解析二进制 cause 字段（递归解析 cause 子节点）
     * @param context 目标上下文
     * @param data 数据视图
     * @param offset 当前偏移量（解析成功时递增）
     * @param depth 当前递归深度
     * @return bool true=成功，false=格式错误或超出深度限制
     */
    bool error_context_serializer_t::parse_binary_cause_field_(
        error_context_t& context, std::string_view data, size_t& offset, size_t depth) noexcept {
        uint8_t has_cause = 0;
        if (!read_byte(data, offset, has_cause)) {
            return false;
        }
        if (has_cause == 0) {
            return true;
        }
        if (depth >= MAX_CAUSE_DEPTH) {
            return false;
        }
        std::string cause_blob;
        if (!read_string_len_prefixed(data, offset, cause_blob)) {
            return false;
        }
        size_t cause_offset = 0;
        auto cause_ctx = from_binary_node_(cause_blob, cause_offset, depth + 1);
        if (!cause_ctx) {
            return false;
        }
        if (cause_offset != cause_blob.size()) {
            return false;
        }
        try {
            context.cause_ = std::make_unique<error_context_t>(std::move(*cause_ctx));
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[error_context_serializer] parse_binary_cause_field_: cause make_unique failed");
            return false;
        }
        return true;
    }

    std::optional<error_context_t> error_context_serializer_t::from_binary_node_(
        std::string_view data, size_t& offset, size_t depth) noexcept {
        if (depth >= MAX_CAUSE_DEPTH) {
            return std::nullopt;
        }
        error_context_t context;

        uint64_t raw_code = 0;
        if (!read_little_endian(data, offset, raw_code)) {
            return std::nullopt;
        }
        context.code_ = error_code_t{raw_code};

        std::string message;
        if (!read_string_len_prefixed(data, offset, message)) {
            return std::nullopt;
        }
        if (!message.empty()) {
            context.ensure_block_();
            if (context.block_) {
                context.block_->message = std::move(message);
            }
        }

        if (auto info = error_registry_t::instance().get_info_cached(context.code_)) {
            context.ensure_block_();
            if (context.block_) {
                context.block_->metadata = std::move(info);
            }
        }

        if (!parse_binary_location_field_(context, data, offset)) {
            return std::nullopt;
        }

        if (!parse_binary_payload_field_(context, data, offset)) {
            return std::nullopt;
        }

        if (!parse_binary_cause_field_(context, data, offset, depth)) {
            return std::nullopt;
        }

        return context;
    }

}  // namespace error_system::core
