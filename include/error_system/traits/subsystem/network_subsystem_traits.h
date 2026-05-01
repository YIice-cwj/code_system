#pragma once
#include "error_system/subsystem/network_subsystem.h"
#include "error_system/traits/subsystem_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct subsystem_traits<subsystem::network_subsystem_t, void> {

        using subsystem_t = subsystem::network_subsystem_t;
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
            return value >= to_int(subsystem_t::none) && value <= to_int(subsystem_t::firewall);
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
                case subsystem_t::tcp:
                    return "tcp";
                case subsystem_t::udp:
                    return "udp";
                case subsystem_t::http:
                    return "http";
                case subsystem_t::https:
                    return "https";
                case subsystem_t::websocket:
                    return "websocket";
                case subsystem_t::grpc:
                    return "grpc";
                case subsystem_t::dns:
                    return "dns";
                case subsystem_t::ssl_tls:
                    return "ssl_tls";
                case subsystem_t::ftp:
                    return "ftp";
                case subsystem_t::smtp:
                    return "smtp";
                case subsystem_t::icmp:
                    return "icmp";
                case subsystem_t::quic:
                    return "quic";
                case subsystem_t::sctp:
                    return "sctp";
                case subsystem_t::bluetooth:
                    return "bluetooth";
                case subsystem_t::zigbee:
                    return "zigbee";
                case subsystem_t::mqtt:
                    return "mqtt";
                case subsystem_t::coap:
                    return "coap";
                case subsystem_t::ntp:
                    return "ntp";
                case subsystem_t::radius:
                    return "radius";
                case subsystem_t::vpn:
                    return "vpn";
                case subsystem_t::sdn:
                    return "sdn";
                case subsystem_t::cdn:
                    return "cdn";
                case subsystem_t::proxy:
                    return "proxy";
                case subsystem_t::firewall:
                    return "firewall";
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
                case utils::string_utils_t::hash("tcp"):
                    return subsystem_t::tcp;
                case utils::string_utils_t::hash("udp"):
                    return subsystem_t::udp;
                case utils::string_utils_t::hash("http"):
                    return subsystem_t::http;
                case utils::string_utils_t::hash("https"):
                    return subsystem_t::https;
                case utils::string_utils_t::hash("websocket"):
                    return subsystem_t::websocket;
                case utils::string_utils_t::hash("grpc"):
                    return subsystem_t::grpc;
                case utils::string_utils_t::hash("dns"):
                    return subsystem_t::dns;
                case utils::string_utils_t::hash("ssl_tls"):
                    return subsystem_t::ssl_tls;
                case utils::string_utils_t::hash("ftp"):
                    return subsystem_t::ftp;
                case utils::string_utils_t::hash("smtp"):
                    return subsystem_t::smtp;
                case utils::string_utils_t::hash("icmp"):
                    return subsystem_t::icmp;
                case utils::string_utils_t::hash("quic"):
                    return subsystem_t::quic;
                case utils::string_utils_t::hash("sctp"):
                    return subsystem_t::sctp;
                case utils::string_utils_t::hash("bluetooth"):
                    return subsystem_t::bluetooth;
                case utils::string_utils_t::hash("zigbee"):
                    return subsystem_t::zigbee;
                case utils::string_utils_t::hash("mqtt"):
                    return subsystem_t::mqtt;
                case utils::string_utils_t::hash("coap"):
                    return subsystem_t::coap;
                case utils::string_utils_t::hash("ntp"):
                    return subsystem_t::ntp;
                case utils::string_utils_t::hash("radius"):
                    return subsystem_t::radius;
                case utils::string_utils_t::hash("vpn"):
                    return subsystem_t::vpn;
                case utils::string_utils_t::hash("sdn"):
                    return subsystem_t::sdn;
                case utils::string_utils_t::hash("cdn"):
                    return subsystem_t::cdn;
                case utils::string_utils_t::hash("proxy"):
                    return subsystem_t::proxy;
                case utils::string_utils_t::hash("firewall"):
                    return subsystem_t::firewall;
                default:
                    return subsystem_t::none;
            }
        }
    };
}  // namespace error_system::traits
