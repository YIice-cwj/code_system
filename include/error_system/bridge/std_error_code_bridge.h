#pragma once
#include <cerrno>
#include <cstring>
#include <string>
#include <system_error>

#include "error_system/core/error_code.h"
#include "error_system/core/error_level.h"
#include "error_system/domain/system_domain.h"

/**
 * @file std_error_code_bridge.h
 * @brief error_code_t 与 std::error_code / errno 的双向桥接
 * @details 提供与 C++ 标准库 <system_error> 体系的互操作能力，使 error_code_t
 *          能够融入标准库生态（std::system_error、std::generic_category 等）。
 *
 *          本头文件为可选包含，不污染 core 头文件依赖关系：
 *          仅当用户需要与 <system_error> 互操作时才引入。
 *
 *          桥接策略：
 *          - error_code_t → std::error_code：使用 error_system_category() 自定义类别，
 *            message() 返回 identity code 的十六进制表示，避免与通用 category 冲突
 *          - errno → error_code_t：映射为 system 域 / system 子系统 / error 级别
 *          - 成功码统一映射为 std::error_code{}（默认构造表示无错误）
 *
 *          所有接口 noexcept，内部使用 try-catch 兜底 std::string 分配失败。
 * @author yiice
 * @version 1.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */
namespace error_system::bridge {

    using error_system::core::code_t;
    using error_system::core::error_code_t;
    using error_system::core::error_level_t;
    using error_system::domain::system_domain_t;

    /**
     * @brief error_system 自定义 error_category 单例
     * @details 与 std::generic_category / std::system_category 并列的独立类别，
     *          用于承载 error_code_t 的 64 位结构化信息。name() 返回固定字符串 "error_system"，
     *          message() 返回 identity code 的十六进制字符串，避免在 category 内部
     *          反查 registry（保持 category 无状态、无锁、可跨 TU 稳定）。
     */
    class error_system_category_t : public std::error_category {
    public:
        /**
         * @brief 获取类别名称
         * @return const char* 固定字符串 "error_system"
         */
        [[nodiscard]] const char* name() const noexcept override { return "error_system"; }

        /**
         * @brief 根据 ev 生成错误描述
         * @param ev 由 to_std_error_code 转换得到的 error_code_t identity 值
         * @return std::string 形如 "error_system:0x0011223344556677" 的描述
         *
         * 实现思路：将 ev 视为 code_t，格式化为十六进制表示。
         *           noexcept 约束下，bad_alloc 时返回静态回退字符串。
         */
        [[nodiscard]] std::string message(int ev) const noexcept override {
            try {
                char buf[64]{};
                std::snprintf(buf, sizeof(buf), "error_system:0x%016llx",
                              static_cast<unsigned long long>(static_cast<code_t>(ev)));
                return std::string(buf);
            } catch (...) {
                return std::string{"error_system:unknown"};
            }
        }

        /**
         * @brief 等价性判断（与标准 default_equivalent 保持一致）
         * @details 仅当 code 域同属 error_system_category 时比较 identity 值，
         *          跨类别返回 false，遵循 std::error_code 默认等价语义。
         */
        [[nodiscard]] bool equivalent(int code, const std::error_condition& cond) const noexcept override {
            return std::error_category::equivalent(code, cond);
        }

        /**
         * @brief 与其他 category 的等价判断（默认实现）
         */
        [[nodiscard]] bool equivalent(const std::error_code& code, int condition) const noexcept override {
            return std::error_category::equivalent(code, condition);
        }
    };

    /**
     * @brief 获取 error_system_category 单例引用
     * @return const std::error_category& 单例引用，跨 TU 稳定
     *
     * 实现思路：函数局部静态，避免 SIOF，初始化线程安全由 C++11 标准保证。
     */
    inline const std::error_category& error_system_category() noexcept {
        static error_system_category_t instance{};
        return instance;
    }

    /**
     * @brief errno 值转换为 error_code_t
     * @param err_no POSIX errno 值（如 EINVAL、ENOENT、ETIMEDOUT）
     * @return error_code_t 映射为 system 域 / error 级，number 携带 errno 原值
     *
     * 实现思路：errno 是 POSIX 系统调用错误的标准载体，统一映射到 system 域，
     *           subsystem=0、module=0，number 字段直接携带 errno 数值（≤65535 范围内）。
     *           retryable 标志对常见可重试 errno（EAGAIN/EWOULDBLOCK/ETIMEDOUT/EINTR）置位。
     */
    [[nodiscard]] inline error_code_t from_errno(int err_no) noexcept {
        const uint16_t number = static_cast<uint16_t>(static_cast<unsigned int>(err_no) & 0xFFFFu);
        error_code_t code{error_level_t::error,
                          system_domain_t::system,
                          core::subsystem_id_t{0},
                          core::module_id_t{0},
                          core::error_number_t{number}};
        if (err_no == EAGAIN || err_no == EWOULDBLOCK || err_no == ETIMEDOUT || err_no == EINTR) {
            code.set_retryable(true);
            code.set_transient(true);
        }
        return code;
    }

    /**
     * @brief error_code_t 转换为 errno 值
     * @param code 业务错误码
     * @return int 成功码返回 0；error_system_category 反解得到的 errno 原值；
     *         其他错误码返回 EIO 作为通用失败回退
     *
     * 实现思路：只有来自 system 域且 subsystem=0/module=0 的错误码才视为 errno 来源，
     *           反解 number 字段作为 errno；其他错误码无对应 POSIX errno，回退 EIO。
     */
    [[nodiscard]] inline int to_errno(error_code_t code) noexcept {
        if (code.is_success_code()) {
            return 0;
        }
        if (code.get_system() == system_domain_t::system
            && code.get_subsys() == 0
            && code.get_module() == 0) {
            return static_cast<int>(code.get_number());
        }
        return EIO;
    }

    /**
     * @brief error_code_t 转换为 std::error_code
     * @param code 业务错误码
     * @return std::error_code 成功码返回默认构造（无错误），错误码用 error_system_category 承载
     *
     * 实现思路：成功码直接返回 std::error_code{}（标准约定无错误）；
     *           错误码将 identity（清除 sign/reserved 位）作为 int 携带，类别用 error_system_category。
     *           int 截断风险：identity 高位 sign/reserved 已清零，剩余 62 位仍可能超出 int 范围，
     *           此处用 static_cast 截断为 int 是 category 协议限制（int 承载 value），
     *           完整 64 位信息通过 message() 文本回读，调用方需要原始 code 应保留 error_code_t。
     */
    [[nodiscard]] inline std::error_code to_std_error_code(error_code_t code) noexcept {
        if (code.is_success_code()) {
            return std::error_code{};
        }
        const code_t identity = code.get_identity_code();
        return std::error_code{static_cast<int>(static_cast<int64_t>(identity)),
                               error_system_category()};
    }

    /**
     * @brief std::error_code 转换为 error_code_t
     * @param ec 标准错误码
     * @return error_code_t 默认 category 或 error_system_category 反查回原始 identity；
     *         其他 category（generic/system）走 errno 桥接路径
     *
     * 实现思路：
     *   - 默认构造（无错误）→ 成功码
     *   - error_system_category → 从 int 反解 identity，重设 sign=0（错误）
     *   - std::generic_category → 调用 from_errno(ec.value())
     *   - std::system_category → 调用 from_errno(ec.value())
     *   - 其他未知 category → 退化构造一个 system 域 error 级错误码，number 携带 ec.value() 截断
     */
    [[nodiscard]] inline error_code_t from_std_error_code(const std::error_code& ec) noexcept {
        if (!ec) {
            return error_code_t::make_success();
        }
        const std::error_category& cat = ec.category();
        if (&cat == &error_system_category()) {
            error_code_t reconstructed{static_cast<code_t>(static_cast<int64_t>(ec.value()))};
            reconstructed.set_sign(0);
            return reconstructed;
        }
        if (&cat == &std::generic_category() || &cat == &std::system_category()) {
            return from_errno(ec.value());
        }
        return error_code_t{error_level_t::error,
                            system_domain_t::system,
                            core::subsystem_id_t{0},
                            core::module_id_t{0},
                            core::error_number_t{static_cast<uint16_t>(static_cast<unsigned int>(ec.value()) & 0xFFFFu)}};
    }

    /**
     * @brief 便捷函数：抛出携带 error_code_t 信息的 std::system_error
     * @param code 业务错误码
     * @param what_msg 异常 what() 附加消息
     *
     * 实现思路：在库边界需要异常时（如与 <system_error> 交互的旧代码），
     *           将 error_code_t 转为 std::error_code 后构造 std::system_error。
     *           注意：本函数会抛异常，仅在库边界且必要时使用，符合规范第 14 条。
     */
    [[noreturn]] inline void throw_system_error(error_code_t code, std::string_view what_msg) {
        throw std::system_error{to_std_error_code(code), std::string{what_msg}};
    }

}  // namespace error_system::bridge
