#pragma once
#include <cstddef>
#include <string>
#include <vector>

#if defined(__APPLE__) || defined(__linux__)
#include <cxxabi.h>
#include <execinfo.h>
#elif defined(_WIN32) || defined(_WIN64)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dbghelp.h>
#include <mutex>
#include <sstream>
#endif
#if defined(__MINGW32__) || defined(__MINGW64__)
#include <cxxabi.h>
#endif

/**
 * @file stack_trace_utils.h
 * @brief 堆栈跟踪工具
 * @details 提供调用栈抓取与符号化能力。采用"延迟符号化"设计：
 *          - capture() 仅抓取原始栈帧指针（~5-10 μs），构造时调用
 *          - resolve() 按需符号化（带 thread_local LRU 缓存），输出时调用
 *          - generate() = capture() + resolve()，保留兼容旧调用
 * @author yiice
 * @version 4.1.0
 * @date 2026-07-06
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief 堆栈跟踪工具类
     * @details 不可实例化，所有方法为静态。跨平台支持 POSIX（backtrace + dladdr +
     *          cxxabi）与 Windows（CaptureStackBackTrace + DbgHelp）。
     *
     *          符号缓存：resolve() 内部维护 thread_local LRU 缓存（容量 256），
     *          相同地址二次解析 O(1)。缓存按地址 key，因同一进程内函数地址稳定。
     */
    class stack_trace_utils_t {
    public:
        /**
         * @brief 符号缓存统计
         * @details 仅调试/测试用。包含命中次数、未命中次数与清空次数三个字段
         */
        struct cache_stats_t {
            size_t hits{0};
            size_t misses{0};
            size_t evictions{0};
        };

        stack_trace_utils_t() = delete;
        ~stack_trace_utils_t() noexcept = delete;
        stack_trace_utils_t(const stack_trace_utils_t&) = delete;
        stack_trace_utils_t& operator=(const stack_trace_utils_t&) = delete;
        stack_trace_utils_t(stack_trace_utils_t&&) = delete;
        stack_trace_utils_t& operator=(stack_trace_utils_t&&) = delete;

        /**
         * @brief 抓取当前线程的原始栈帧指针（不做符号化）
         * @param skip_frames 跳过顶部栈帧数（如跳过本函数自身）
         * @param max_frames 最大抓取深度（硬上限 32）
         * @return std::vector<void*> 栈帧指针数组，失败返回空
         * @note 典型开销 5-10 μs，适合构造时调用
         */
        [[nodiscard]] static std::vector<void*> capture(int skip_frames = 1, int max_frames = 16) noexcept;

        /**
         * @brief 符号化栈帧指针数组
         * @param frames 栈帧指针数组首地址
         * @param count 栈帧数量
         * @return std::vector<std::string> 每帧的可读字符串
         * @note 内部使用 thread_local LRU 缓存，相同地址二次解析 O(1)
         */
        [[nodiscard]] static std::vector<std::string> resolve(const void* const* frames, size_t count) noexcept;

        /**
         * @brief 便捷重载：符号化 vector
         * @param frames 栈帧指针数组
         * @return std::vector<std::string> 每帧的可读字符串
         */
        [[nodiscard]] static std::vector<std::string> resolve(const std::vector<void*>& frames) noexcept;

        /**
         * @brief 抓取并立即符号化（= capture + resolve，兼容旧接口）
         * @param skip_frames 跳过顶部栈帧数
         * @param max_frames 最大抓取深度
         * @return std::vector<std::string> 每帧的可读字符串
         */
        [[nodiscard]] static std::vector<std::string> generate(int skip_frames = 1, int max_frames = 16) noexcept;

        /**
         * @brief 清除当前线程的符号缓存
         * @note 仅用于测试与显式刷新场景
         */
        static void clear_symbol_cache() noexcept;

        /**
         * @brief 获取当前线程的符号缓存统计
         * @return cache_stats_t 缓存统计
         */
        [[nodiscard]] static cache_stats_t get_cache_stats() noexcept;
    };

}  // namespace error_system::utils
