#pragma once
#include <cstdint>
#include <type_traits>

#include "error_system/utils/string_utils.h"

/**
 * @file error_level.h
 * @brief 错误等级定义与转换
 * @details 定义错误系统的等级分类（debug/info/warn/error/fatal），
 *          提供错误等级与整数、字符串之间的相互转换功能，
 *          支持编译期常量计算，用于日志过滤和错误处理决策
 * @author yiice
 * @version 3.0.0
 * @date 2026-04-27
 * @copyright Copyright (c) 2026
 */
namespace error_system::core {

    /**
     * @brief 错误等级分类
     * @details 定义错误系统的等级分类（debug/info/warn/error/fatal)
     *          用于表示错误等级的严重程度和处理优先级
     */
    enum class error_level_t : uint8_t {
        debug = 0,    // 调试
        info = 1,     // 信息
        warn = 2,     // 警告
        error = 3,    // 错误
        fatal = 4,    // 致命错误
    };

    /**
     * @brief 错误等级字符串
     * @details 用于表示错误等级的字符串
     *          与错误等级分类一一对应，用于日志打印和错误处理
     */
    inline constexpr const char* ERROR_LEVEL_STRING[] = {"debug", "info", "warn", "error", "fatal"};

    /**
     * @brief 错误等级整数
     * @details 用于将错误等级转换为错误等级整数
     * @param level 错误等级
     * @return uint8_t 错误等级整数
     */
    [[nodiscard]] constexpr uint8_t to_int(error_level_t level) noexcept {
        return static_cast<uint8_t>(std::underlying_type_t<error_level_t>(level));
    }

    /**
     * @brief 错误等级整数是否有效
     * @details 用于判断错误等级整数是否有效
     * @param level 错误等级整数
     * @return bool 错误等级整数是否有效
     */
    [[nodiscard]] constexpr bool is_valid(uint8_t level) noexcept {
        return level <= to_int(error_level_t::fatal);
    }

    /**
     * @brief 错误等级整数转换为错误等级
     * @details 用于将错误等级整数转换为错误等级
     * @param level 错误等级整数
     * @return error_level_t 错误等级
     */
    [[nodiscard]] constexpr error_level_t from_int(uint8_t level) noexcept {
        if (!is_valid(level)) {
            return error_level_t::fatal;
        }
        return static_cast<error_level_t>(level);
    }

    /**
     * @brief 错误等级字符串转换为错误等级
     * @details 先按 FNV-1a 哈希快速匹配，匹配后再做字符串二次校验以规避哈希碰撞导致的误判。
     *          string 为 nullptr 时返回 info。
     * @param string 错误等级字符串
     * @return error_level_t 错误等级
     */
    [[nodiscard]] constexpr error_level_t from_string(const char* string) noexcept {
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

    /**
     * @brief 错误等级字符串
     * @details 用于将错误等级转换为错误等级字符串。
     *          内置范围校验，非法值返回 "unknown"，避免数组越界
     * @param level 错误等级
     * @return const char* 错误等级字符串
     */
    [[nodiscard]] constexpr const char* to_string(error_level_t level) noexcept {
        const uint8_t idx = to_int(level);
        if (!is_valid(idx)) {
            return "unknown";
        }
        return ERROR_LEVEL_STRING[idx];
    }

    /**
     * @brief 错误等级整数的下一个错误等级
     * @details 用于获取错误等级整数的下一个错误等级。超过 fatal 时 clamp 为 fatal
     * @param level 错误等级整数
     * @return error_level_t 错误等级整数的下一个错误等级
     */
    [[nodiscard]] constexpr error_level_t next_level(error_level_t level) noexcept {
        return from_int(to_int(level) + 1);
    }

    /**
     * @brief 错误等级整数的上一个错误等级
     * @details 用于获取错误等级整数的上一个错误等级。debug 下溢时回绕为 fatal（无更低级别）
     * @param level 错误等级整数
     * @return error_level_t 错误等级整数的上一个错误等级
     */
    [[nodiscard]] constexpr error_level_t prev_level(error_level_t level) noexcept {
        const uint8_t idx = to_int(level);
        return idx == 0 ? error_level_t::fatal : from_int(idx - 1);
    }

    /**
     * @brief 错误等级整数是否大于等于最小错误等级
     * @details 用于判断错误等级整数是否大于等于最小错误等级
     * @param current 错误等级整数
     * @param min_level 最小错误等级整数
     * @return bool 错误等级整数是否大于等于最小错误等级
     */
    [[nodiscard]] constexpr bool should_log(error_level_t current, error_level_t min_level) noexcept {
        return to_int(current) >= to_int(min_level);
    }

}  // namespace error_system::core