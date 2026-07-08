#pragma once
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <string>
#include <string_view>
#include <thread>

#include "error_system/utils/source_location.h"
#include "error_system/utils/string_format.h"

/**
 * @file log.h
 * @brief 统一日志工具
 * @details 提供分级日志输出（DEBUG/INFO/WARN/ERROR/FATAL），格式包含时间戳、
 *          日志级别、文件名、行号、线程 ID，符合规范第 15 条。
 *          所有函数 noexcept 安全，输出目标为 stderr。
 *
 *          用法一（纯字符串，零分配）：
 *          @code
 *          LOG_ERROR("[error_context] wrap: cycle detected, skipping cause");
 *          @endcode
 *
 *          用法二（{} 格式化，委托 string_format_t::format）：
 *          @code
 *          LOG_ERROR("[error_router_plugin] handler exception: {}", e.what());
 *          @endcode
 *
 *          宏自动捕获 __FILE__ / __LINE__，调用方无需手动传递位置。
 *          日志行尾自动追加换行，调用方无需写 \n。
 * @author yiice
 * @version 4.4.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief 日志级别
     * @details 从低到高：debug < info < warn < error < fatal
     */
    enum class log_level_t : uint8_t {
        debug,
        info,
        warn,
        error,
        fatal,
    };

    /**
     * @brief 日志级别名称
     * @param level 日志级别
     * @return 级别名称字符串（DEBUG/INFO/WARN/ERROR/FATAL）
     */
    [[nodiscard]] inline const char* log_level_name(log_level_t level) noexcept {
        switch (level) {
            case log_level_t::debug: return "DEBUG";
            case log_level_t::info:  return "INFO";
            case log_level_t::warn:  return "WARN";
            case log_level_t::error: return "ERROR";
            case log_level_t::fatal: return "FATAL";
        }
        return "?";
    }

    /**
     * @brief 输出日志前缀并写入消息体
     * @details 前缀格式：[时间戳] [级别] [文件:行号] [tid:线程ID] 消息\n
     * @param level 日志级别
     * @param file 源文件路径（自动提取短文件名）
     * @param line 行号
     * @param message 已格式化的消息内容
     */
    inline void log_write(log_level_t level, const char* file, int line, std::string_view message) noexcept {
        std::time_t now = std::time(nullptr);
        std::tm tm_buf{};
#if defined(_WIN32)
        localtime_s(&tm_buf, &now);
#else
        localtime_r(&now, &tm_buf);
#endif
        char time_buf[24];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm_buf);
        std::fprintf(stderr, "[%s] [%s] [%s:%d] [tid:%llu] ",
                     time_buf,
                     log_level_name(level),
                     extract_short_filename(file),
                     line,
                     static_cast<unsigned long long>(
                         std::hash<std::thread::id>{}(std::this_thread::get_id())));
        std::fwrite(message.data(), 1, message.size(), stderr);
        std::fputc('\n', stderr);
    }

    /**
     * @brief 输出日志（纯字符串，零分配）
     * @param level 日志级别
     * @param file 源文件路径
     * @param line 行号
     * @param message 日志消息（不含换行符）
     */
    inline void log(log_level_t level, const char* file, int line, const char* message) noexcept {
        log_write(level, file, line, std::string_view(message));
    }

    /**
     * @brief 输出日志（{} 格式化，委托 string_format_t::format）
     * @tparam Args 格式化参数类型
     * @param level 日志级别
     * @param file 源文件路径
     * @param line 行号
     * @param fmt {} 风格格式字符串
     * @param args 格式化参数
     */
    template <typename... Args>
    inline void log(log_level_t level, const char* file, int line, std::string_view fmt, Args&&... args) noexcept {
        log_write(level, file, line, string_format_t::format(fmt, std::forward<Args>(args)...));
    }

}  // namespace error_system::utils

/**
 * @brief ERROR 级别日志宏
 * @details 自动捕获 __FILE__ / __LINE__。支持纯字符串和 {} 格式化两种用法。
 */
#define LOG_ERROR(...) ::error_system::utils::log(::error_system::utils::log_level_t::error, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief WARN 级别日志宏
 */
#define LOG_WARN(...) ::error_system::utils::log(::error_system::utils::log_level_t::warn, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief INFO 级别日志宏
 */
#define LOG_INFO(...) ::error_system::utils::log(::error_system::utils::log_level_t::info, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief DEBUG 级别日志宏
 */
#define LOG_DEBUG(...) ::error_system::utils::log(::error_system::utils::log_level_t::debug, __FILE__, __LINE__, __VA_ARGS__)

/**
 * @brief FATAL 级别日志宏
 */
#define LOG_FATAL(...) ::error_system::utils::log(::error_system::utils::log_level_t::fatal, __FILE__, __LINE__, __VA_ARGS__)
