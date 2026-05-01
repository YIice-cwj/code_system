#pragma once
#include "error_system/module/network_module.h"
#include "error_system/traits/module_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct module_traits<module::network_module_t, void> {

        using module_t = module::network_module_t;
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
            return value >= to_int(module_t::none) && value <= to_int(module_t::tunnel);
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
                case module_t::socket:
                    return "socket";
                case module_t::listener:
                    return "listener";
                case module_t::acceptor:
                    return "acceptor";
                case module_t::connector:
                    return "connector";
                case module_t::session:
                    return "session";
                case module_t::channel:
                    return "channel";
                case module_t::pipeline:
                    return "pipeline";
                case module_t::handshake:
                    return "handshake";
                case module_t::heartbeat:
                    return "heartbeat";
                case module_t::reconnection:
                    return "reconnection";
                case module_t::timeout:
                    return "timeout";
                case module_t::dns_resolver:
                    return "dns_resolver";
                case module_t::ssl_context:
                    return "ssl_context";
                case module_t::packet_encoder:
                    return "packet_encoder";
                case module_t::packet_decoder:
                    return "packet_decoder";
                case module_t::congestion:
                    return "congestion";
                case module_t::flow_control:
                    return "flow_control";
                case module_t::nat:
                    return "nat";
                case module_t::acl:
                    return "acl";
                case module_t::qos:
                    return "qos";
                case module_t::multicast:
                    return "multicast";
                case module_t::broadcast:
                    return "broadcast";
                case module_t::proxy_forward:
                    return "proxy_forward";
                case module_t::tunnel:
                    return "tunnel";
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
                case utils::string_utils_t::hash("socket"):
                    return module_t::socket;
                case utils::string_utils_t::hash("listener"):
                    return module_t::listener;
                case utils::string_utils_t::hash("acceptor"):
                    return module_t::acceptor;
                case utils::string_utils_t::hash("connector"):
                    return module_t::connector;
                case utils::string_utils_t::hash("session"):
                    return module_t::session;
                case utils::string_utils_t::hash("channel"):
                    return module_t::channel;
                case utils::string_utils_t::hash("pipeline"):
                    return module_t::pipeline;
                case utils::string_utils_t::hash("handshake"):
                    return module_t::handshake;
                case utils::string_utils_t::hash("heartbeat"):
                    return module_t::heartbeat;
                case utils::string_utils_t::hash("reconnection"):
                    return module_t::reconnection;
                case utils::string_utils_t::hash("timeout"):
                    return module_t::timeout;
                case utils::string_utils_t::hash("dns_resolver"):
                    return module_t::dns_resolver;
                case utils::string_utils_t::hash("ssl_context"):
                    return module_t::ssl_context;
                case utils::string_utils_t::hash("packet_encoder"):
                    return module_t::packet_encoder;
                case utils::string_utils_t::hash("packet_decoder"):
                    return module_t::packet_decoder;
                case utils::string_utils_t::hash("congestion"):
                    return module_t::congestion;
                case utils::string_utils_t::hash("flow_control"):
                    return module_t::flow_control;
                case utils::string_utils_t::hash("nat"):
                    return module_t::nat;
                case utils::string_utils_t::hash("acl"):
                    return module_t::acl;
                case utils::string_utils_t::hash("qos"):
                    return module_t::qos;
                case utils::string_utils_t::hash("multicast"):
                    return module_t::multicast;
                case utils::string_utils_t::hash("broadcast"):
                    return module_t::broadcast;
                case utils::string_utils_t::hash("proxy_forward"):
                    return module_t::proxy_forward;
                case utils::string_utils_t::hash("tunnel"):
                    return module_t::tunnel;
                default:
                    return module_t::none;
            }
        }
    };
}  // namespace error_system::traits
