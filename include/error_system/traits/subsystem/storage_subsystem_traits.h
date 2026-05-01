#pragma once
#include "error_system/subsystem/storage_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::storage_subsystem_t, void> {

        using subsystem_t = subsystem::storage_subsystem_t;
        using underlying_t = std::underlying_type_t<subsystem_t>;

        /**
         * @brief 将枚举值转换为整数
         * @param subsystem 枚举值
         * @return 对应的整数值
         */
        static constexpr underlying_t to_int(subsystem_t subsystem) noexcept {
            return static_cast<underlying_t>(subsystem);
        }

        /**
         * @brief 校验整数是否为有效的枚举值
         * @param value 要校验的整数
         * @return 如果是有效的枚举值，返回 true；否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::block);
        }

        /**
         * @brief 从整数转换为枚举值
         * @param value 要转换的整数
         * @return 对应的枚举值，如果无效则返回 subsystem_t::none
         */
        static constexpr subsystem_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return subsystem_t::none;
            }
            return static_cast<subsystem_t>(value);
        }

        /**
         * @brief 将枚举值转换为字符串
         * @param subsystem 枚举值
         * @return 对应的字符串
         */
        static constexpr const char* to_string(subsystem_t subsystem) noexcept {
            switch (subsystem) {
                case subsystem_t::none:
                    return "none";
                case subsystem_t::local:
                    return "local";
                case subsystem_t::s3:
                    return "s3";
                case subsystem_t::oss:
                    return "oss";
                case subsystem_t::cos:
                    return "cos";
                case subsystem_t::hdfs:
                    return "hdfs";
                case subsystem_t::nfs:
                    return "nfs";
                case subsystem_t::ceph:
                    return "ceph";
                case subsystem_t::minio:
                    return "minio";
                case subsystem_t::azure_blob:
                    return "azure_blob";
                case subsystem_t::gcs:
                    return "gcs";
                case subsystem_t::glusterfs:
                    return "glusterfs";
                case subsystem_t::lustre:
                    return "lustre";
                case subsystem_t::san:
                    return "san";
                case subsystem_t::nas:
                    return "nas";
                case subsystem_t::tape:
                    return "tape";
                case subsystem_t::optical:
                    return "optical";
                case subsystem_t::block:
                    return "block";
                default:
                    return "unknown";
            }
        }

        /**
         * @brief 从字符串转换为枚举值
         * @param string 要转换的字符串
         * @return 对应的枚举值，如果无效则返回 subsystem_t::none
         */
        static constexpr subsystem_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
                case utils::string_utils_t::hash("none"):
                    return subsystem_t::none;
                case utils::string_utils_t::hash("local"):
                    return subsystem_t::local;
                case utils::string_utils_t::hash("s3"):
                    return subsystem_t::s3;
                case utils::string_utils_t::hash("oss"):
                    return subsystem_t::oss;
                case utils::string_utils_t::hash("cos"):
                    return subsystem_t::cos;
                case utils::string_utils_t::hash("hdfs"):
                    return subsystem_t::hdfs;
                case utils::string_utils_t::hash("nfs"):
                    return subsystem_t::nfs;
                case utils::string_utils_t::hash("ceph"):
                    return subsystem_t::ceph;
                case utils::string_utils_t::hash("minio"):
                    return subsystem_t::minio;
                case utils::string_utils_t::hash("azure_blob"):
                    return subsystem_t::azure_blob;
                case utils::string_utils_t::hash("gcs"):
                    return subsystem_t::gcs;
                case utils::string_utils_t::hash("glusterfs"):
                    return subsystem_t::glusterfs;
                case utils::string_utils_t::hash("lustre"):
                    return subsystem_t::lustre;
                case utils::string_utils_t::hash("san"):
                    return subsystem_t::san;
                case utils::string_utils_t::hash("nas"):
                    return subsystem_t::nas;
                case utils::string_utils_t::hash("tape"):
                    return subsystem_t::tape;
                case utils::string_utils_t::hash("optical"):
                    return subsystem_t::optical;
                case utils::string_utils_t::hash("block"):
                    return subsystem_t::block;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
