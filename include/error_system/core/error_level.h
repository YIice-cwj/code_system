#pragma once
#include <cstdint>
#include <type_traits>

#include "error_system/utils/string_utils.h"

/**
 * @file error_level.h
 * @brief 错误等级定义与转换
 * @details 定义错误系统的五级分类（debug/info/warn/error/fatal），
 *          提供与整数、字符串之间的相互转换，支持编译期常量计算。
 *          用于日志过滤和错误处理决策。
 * @author yiice
 * @version 3.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 错误等级分类
     * @details 表示错误的严重程度和处理优先级，数值越大越严重。
     */
    enum class error_level_t : uint8_t {
        debug = 0,
        info = 1,
        warn = 2,
        error = 3,
        fatal = 4,
    };

    /**
     * @brief 错误等级字符串表（与 error_level_t 一一对应）
     */
    inline constexpr const char* ERROR_LEVEL_STRING[] = {"debug", "info", "warn", "error", "fatal"};

    /**
     * @brief 错误等级转整数
     * @param level 错误等级
     * @return 等级整数值
     */
    [[nodiscard]] constexpr uint8_t to_int(error_level_t level) noexcept;

    /**
     * @brief 整数是否为有效错误等级
     * @param level 等级整数
     * @return 有效返回 true，否则 false
     */
    [[nodiscard]] constexpr bool is_valid(uint8_t level) noexcept;

    /**
     * @brief 整数转错误等级
     * @details 非法值回退为 fatal，避免下游越界。
     * @param level 等级整数
     * @return 错误等级枚举
     */
    [[nodiscard]] constexpr error_level_t from_int(uint8_t level) noexcept;

    /**
     * @brief 字符串转错误等级
     * @details 先按 FNV-1a 哈希快速匹配，再做字符串二次校验规避哈希碰撞。
     *          string 为 nullptr 时返回 info。
     * @param string 等级字符串
     * @return 错误等级枚举
     */
    [[nodiscard]] constexpr error_level_t from_string(const char* string) noexcept;

    /**
     * @brief 错误等级转字符串
     * @details 内置范围校验，非法值返回 "unknown"。
     * @param level 错误等级
     * @return 等级字符串
     */
    [[nodiscard]] constexpr const char* to_string(error_level_t level) noexcept;

    /**
     * @brief 获取下一个错误等级
     * @details 超过 fatal 时 clamp 为 fatal。
     * @param level 当前错误等级
     * @return 下一个错误等级
     */
    [[nodiscard]] constexpr error_level_t next_level(error_level_t level) noexcept;

    /**
     * @brief 获取上一个错误等级
     * @details debug 下溢时回绕为 fatal（无更低级别）。
     * @param level 当前错误等级
     * @return 上一个错误等级
     */
    [[nodiscard]] constexpr error_level_t prev_level(error_level_t level) noexcept;

    /**
     * @brief 判断错误等级是否达到日志输出阈值
     * @param current 当前错误等级
     * @param min_level 最低日志等级
     * @return 达到阈值返回 true，否则 false
     */
    [[nodiscard]] constexpr bool should_log(error_level_t current, error_level_t min_level) noexcept;

    [[nodiscard]] inline constexpr uint8_t to_int(error_level_t level) noexcept {
        return static_cast<uint8_t>(std::underlying_type_t<error_level_t>(level));
    }

    [[nodiscard]] inline constexpr bool is_valid(uint8_t level) noexcept {
        return level <= to_int(error_level_t::fatal);
    }

    [[nodiscard]] inline constexpr error_level_t from_int(uint8_t level) noexcept {
        if (!is_valid(level)) {
            return error_level_t::fatal;
        }
        return static_cast<error_level_t>(level);
    }

    [[nodiscard]] inline constexpr error_level_t from_string(const char* string) noexcept {
        if (string == nullptr) {
            return error_level_t::info;
        }
        const std::string_view sv(string);
        switch (utils::string_utils_t::hash(sv)) {
            case utils::string_utils_t::hash("debug"):
                if (sv == "debug") { return error_level_t::debug; }
                break;
            case utils::string_utils_t::hash("info"):
                if (sv == "info") { return error_level_t::info; }
                break;
            case utils::string_utils_t::hash("warn"):
                if (sv == "warn") { return error_level_t::warn; }
                break;
            case utils::string_utils_t::hash("error"):
                if (sv == "error") { return error_level_t::error; }
                break;
            case utils::string_utils_t::hash("fatal"):
                if (sv == "fatal") { return error_level_t::fatal; }
                break;
            default:
                break;
        }
        return error_level_t::info;
    }

    [[nodiscard]] inline constexpr const char* to_string(error_level_t level) noexcept {
        const uint8_t idx = to_int(level);
        if (!is_valid(idx)) {
            return "unknown";
        }
        return ERROR_LEVEL_STRING[idx];
    }

    [[nodiscard]] inline constexpr error_level_t next_level(error_level_t level) noexcept {
        return from_int(to_int(level) + 1);
    }

    [[nodiscard]] inline constexpr error_level_t prev_level(error_level_t level) noexcept {
        const uint8_t idx = to_int(level);
        return idx == 0 ? error_level_t::fatal : from_int(idx - 1);
    }

    [[nodiscard]] inline constexpr bool should_log(error_level_t current, error_level_t min_level) noexcept {
        return to_int(current) >= to_int(min_level);
    }

}  // namespace error_system::core
