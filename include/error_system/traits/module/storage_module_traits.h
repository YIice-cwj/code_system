#pragma once
#include "error_system/module/storage_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::storage_module_t, void> {

        using module_t = module::storage_module_t;
        using underlying_t = std::underlying_type_t<module_t>;

        /**
         * @brief 将枚举值转换为整数
         * @param module 枚举值
         * @return 对应的整数值
         */
        static constexpr underlying_t to_int(module_t module) noexcept {
            return static_cast<underlying_t>(module);
        }

        /**
         * @brief 校验整数是否为有效的枚举值
         * @param value 要校验的整数
         * @return 如果是有效的枚举值，返回 true；否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(module_t::none) && value <= to_int(module_t::trash);
        }

        /**
         * @brief 从整数转换为枚举值
         * @param value 要转换的整数
         * @return 对应的枚举值，如果无效则返回 module_t::none
         */
        static constexpr module_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return module_t::none;
            }
            return static_cast<module_t>(value);
        }

        /**
         * @brief 将枚举值转换为字符串
         * @param module 枚举值
         * @return 对应的字符串
         */
        static constexpr const char* to_string(module_t module) noexcept {
            switch (module) {
                case module_t::none:
                    return "none";
                case module_t::file_reader:
                    return "file_reader";
                case module_t::file_writer:
                    return "file_writer";
                case module_t::file_manager:
                    return "file_manager";
                case module_t::directory:
                    return "directory";
                case module_t::watcher:
                    return "watcher";
                case module_t::uploader:
                    return "uploader";
                case module_t::downloader:
                    return "downloader";
                case module_t::sync:
                    return "sync";
                case module_t::backup:
                    return "backup";
                case module_t::compressor:
                    return "compressor";
                case module_t::encryptor:
                    return "encryptor";
                case module_t::bucket:
                    return "bucket";
                case module_t::object:
                    return "object";
                case module_t::multipart:
                    return "multipart";
                case module_t::snapshot:
                    return "snapshot";
                case module_t::replication:
                    return "replication";
                case module_t::deduplication:
                    return "deduplication";
                case module_t::tiering:
                    return "tiering";
                case module_t::quota:
                    return "quota";
                case module_t::lifecycle:
                    return "lifecycle";
                case module_t::versioning:
                    return "versioning";
                case module_t::trash:
                    return "trash";
                default:
                    return "unknown";
            }
        }

        /**
         * @brief 从字符串转换为枚举值
         * @param string 要转换的字符串
         * @return 对应的枚举值，如果无效则返回 module_t::none
         */
        static constexpr module_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"):
                    return module_t::none;
                case utils::string_utils_t::hash("file_reader"):
                    return module_t::file_reader;
                case utils::string_utils_t::hash("file_writer"):
                    return module_t::file_writer;
                case utils::string_utils_t::hash("file_manager"):
                    return module_t::file_manager;
                case utils::string_utils_t::hash("directory"):
                    return module_t::directory;
                case utils::string_utils_t::hash("watcher"):
                    return module_t::watcher;
                case utils::string_utils_t::hash("uploader"):
                    return module_t::uploader;
                case utils::string_utils_t::hash("downloader"):
                    return module_t::downloader;
                case utils::string_utils_t::hash("sync"):
                    return module_t::sync;
                case utils::string_utils_t::hash("backup"):
                    return module_t::backup;
                case utils::string_utils_t::hash("compressor"):
                    return module_t::compressor;
                case utils::string_utils_t::hash("encryptor"):
                    return module_t::encryptor;
                case utils::string_utils_t::hash("bucket"):
                    return module_t::bucket;
                case utils::string_utils_t::hash("object"):
                    return module_t::object;
                case utils::string_utils_t::hash("multipart"):
                    return module_t::multipart;
                case utils::string_utils_t::hash("snapshot"):
                    return module_t::snapshot;
                case utils::string_utils_t::hash("replication"):
                    return module_t::replication;
                case utils::string_utils_t::hash("deduplication"):
                    return module_t::deduplication;
                case utils::string_utils_t::hash("tiering"):
                    return module_t::tiering;
                case utils::string_utils_t::hash("quota"):
                    return module_t::quota;
                case utils::string_utils_t::hash("lifecycle"):
                    return module_t::lifecycle;
                case utils::string_utils_t::hash("versioning"):
                    return module_t::versioning;
                case utils::string_utils_t::hash("trash"):
                    return module_t::trash;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
