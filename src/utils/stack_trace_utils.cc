#include "error_system/utils/stack_trace_utils.h"

/**
 * @file stack_trace_utils.cc
 * @brief 堆栈追踪工具实现
 * @details 三阶段优化实现：
 *          1. capture() 仅抓取原始栈帧指针（backtrace / CaptureStackBackTrace）
 *          2. resolve() 按需符号化，内部维护 thread_local LRU 符号缓存
 *          3. POSIX 用 dladdr 逐帧解析替代 backtrace_symbols 一次大 malloc；
 *             Windows 沿用 SymFromAddr（本就是逐帧）
 * @author yiice
 * @version 4.1.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <cstdio>
#include <new>
#include <unordered_map>
#include <utility>

#if defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#endif

#include "error_system/utils/bad_alloc_handler.h"
#include "error_system/utils/log.h"

namespace error_system::utils {

#ifdef ERROR_SYSTEM_ENABLE_STACKTRACE

    namespace {
        constexpr int HARD_MAX_FRAMES = 32;
        constexpr size_t SYMBOL_CACHE_CAPACITY = 256;

        /**
         * @brief thread_local 符号缓存
         * @details 无锁，每线程独立
         */
        struct symbol_cache_t {
            std::unordered_map<void*, std::string> entries;
            size_t hits{0};
            size_t misses{0};
            size_t evictions{0};
        };

        thread_local symbol_cache_t tls_cache{};

#if defined(__APPLE__) || defined(__linux__)

        /**
         * @brief 抓取当前线程的原始栈帧指针
         * @param buffer 栈帧指针缓冲区
         * @param max_frames 最大抓取深度
         * @return int 实际抓取的栈帧数
         */
        int capture_os_frames(void** buffer, int max_frames) noexcept {
            return backtrace(buffer, max_frames);
        }

        /**
         * @brief 用 dladdr 解析单个地址
         * @details 替代 backtrace_symbols 一次大 malloc，逐帧解析更可控。
         *          输出格式：demangled_name + offset（无 dli_fname 前缀，更简洁）。
         *          dladdr 失败时回退为 "[unknown] 0xaddr"。
         *          偏移附加失败时返回已有结果，不影响主流程。
         * @param address 待解析的地址
         * @return std::string 可读的符号字符串
         */
        std::string resolve_single_os_symbol(void* address) noexcept {
            Dl_info info;
            if (dladdr(address, &info) && info.dli_sname) {
                const char* symbol = info.dli_sname;
                std::string result;

                int status = -1;
                char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
                if (status == 0 && demangled) {
                    result = demangled;
                    free(demangled);
                } else {
                    try {
                        result = symbol;
                    } catch (const std::bad_alloc&) {
                        if (demangled) { free(demangled); }
                        return {};
                    }
                }

                const ptrdiff_t offset = static_cast<char*>(address) - static_cast<char*>(info.dli_saddr);
                try {
                    result += " +";
                    result += std::to_string(offset);
                } catch (const std::bad_alloc&) {
                }
                return result;
            }

            char buf[32];
            std::snprintf(buf, sizeof(buf), "%p", address);
            try {
                return std::string("[unknown] ") + buf;
            } catch (const std::bad_alloc&) {
                return {};
            }
        }

#elif defined(_WIN32)

        /**
         * @brief DbgHelp 管理器
         * @details 单例，负责 SymInitialize/SymCleanup 生命周期与互斥访问
         */
        struct dbghelp_manager_t {
            HANDLE process = GetCurrentProcess();
            std::mutex mutex;

            dbghelp_manager_t() noexcept {
                SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
                if (!SymInitialize(process, nullptr, TRUE)) {
                    LOG_ERROR("[stack_trace_utils] SymInitialize failed (error={})",
                              static_cast<unsigned long>(GetLastError()));
                }
            }

            ~dbghelp_manager_t() noexcept { SymCleanup(process); }

            dbghelp_manager_t(const dbghelp_manager_t&) = delete;
            dbghelp_manager_t& operator=(const dbghelp_manager_t&) = delete;
            dbghelp_manager_t(dbghelp_manager_t&&) = delete;
            dbghelp_manager_t& operator=(dbghelp_manager_t&&) = delete;

            /**
             * @brief 获取单例实例
             * @return dbghelp_manager_t& 单例引用
             */
            static dbghelp_manager_t& instance() noexcept {
                static dbghelp_manager_t instance;
                return instance;
            }
        };

        /**
         * @brief 抓取当前线程的原始栈帧指针
         * @param buffer 栈帧指针缓冲区
         * @param max_frames 最大抓取深度
         * @return int 实际抓取的栈帧数
         */
        int capture_os_frames(void** buffer, int max_frames) noexcept {
            return CaptureStackBackTrace(0, max_frames, buffer, nullptr);
        }

        /**
         * @brief 用 SymFromAddr 解析单个地址
         * @details MinGW 环境下尝试对 _Z 前缀符号做 cxxabi 反修饰。
         *          解析失败时回退为 "[Unknown Symbol] at <addr>"
         * @param address 待解析的地址
         * @return std::string 可读的符号字符串
         */
        std::string resolve_single_os_symbol(void* address) noexcept {
            auto& dbghelp = dbghelp_manager_t::instance();
            std::lock_guard<std::mutex> lock(dbghelp.mutex);

            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(buffer);
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            DWORD64 address_value = reinterpret_cast<DWORD64>(address);
            DWORD64 displacement = 0;

            if (SymFromAddr(dbghelp.process, address_value, &displacement, symbol)) {
                std::string symbol_str = symbol->Name;

#if defined(__MINGW32__) || defined(__MINGW64__)
                std::string mangled_name = symbol_str;
                if (mangled_name.rfind("Z", 0) == 0) {
                    mangled_name = "_" + mangled_name;
                }
                if (mangled_name.rfind("_Z", 0) == 0) {
                    int status = -1;
                    char* demangled = abi::__cxa_demangle(mangled_name.c_str(), nullptr, nullptr, &status);
                    if (status == 0 && demangled) {
                        symbol_str = std::string(demangled);
                        free(demangled);
                    }
                }
#endif
                return symbol_str;
            }

            try {
                std::ostringstream oss;
                oss << "[Unknown Symbol] at " << address;
                return oss.str();
            } catch (const std::bad_alloc&) {
                return "[Unknown Symbol]";
            }
        }
#endif

        /**
         * @brief 查询符号缓存，未命中则解析并写入
         * @details 缓存容量超限时整体 clear（符号地址稳定，重建成本可接受，
         *          且避免复杂 LRU 链表开销）。缓存写入失败不影响返回值
         * @param address 待查询的地址
         * @return std::string 符号字符串，解析失败返回空
         */
        std::string query_or_resolve(void* address) noexcept {
            auto it = tls_cache.entries.find(address);
            if (it != tls_cache.entries.end()) {
                ++tls_cache.hits;
                return it->second;
            }

            ++tls_cache.misses;
            std::string resolved = resolve_single_os_symbol(address);
            if (resolved.empty()) {
                return resolved;
            }

            if (tls_cache.entries.size() >= SYMBOL_CACHE_CAPACITY) {
                tls_cache.entries.clear();
                ++tls_cache.evictions;
            }

            try {
                tls_cache.entries.emplace(address, resolved);
            } catch (const std::bad_alloc&) {
            }
            return resolved;
        }
    }  // namespace

    std::vector<void*> stack_trace_utils_t::capture(int skip_frames, int max_frames) noexcept {
        if (max_frames <= 0) {
            return {};
        }
        if (max_frames > HARD_MAX_FRAMES) {
            max_frames = HARD_MAX_FRAMES;
        }

        try {
            std::vector<void*> callstack(static_cast<size_t>(max_frames));
            const int frames = capture_os_frames(callstack.data(), max_frames);
            if (frames <= 0) {
                return {};
            }
            callstack.resize(static_cast<size_t>(frames));

            if (skip_frames > 0) {
                if (skip_frames >= frames) {
                    return {};
                }
                callstack.erase(callstack.begin(),
                                callstack.begin() + skip_frames);
            }
            return callstack;
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("stack_trace_utils", "capture");
            return {};
        }
    }

    std::vector<std::string> stack_trace_utils_t::resolve(const void* const* frames, size_t count) noexcept {
        std::vector<std::string> result;
        if (!frames || count == 0) {
            return result;
        }

        try {
            result.reserve(count);
            for (size_t i = 0; i < count; ++i) {
                result.push_back(query_or_resolve(const_cast<void*>(frames[i])));
            }
        } catch (const std::bad_alloc&) {
            utils::report_bad_alloc("stack_trace_utils", "resolve");
        }
        return result;
    }

    std::vector<std::string> stack_trace_utils_t::resolve(const std::vector<void*>& frames) noexcept {
        if (frames.empty()) {
            return {};
        }
        return resolve(frames.data(), frames.size());
    }

    std::vector<std::string> stack_trace_utils_t::generate(int skip_frames, int max_frames) noexcept {
        std::vector<void*> raw = capture(skip_frames, max_frames);
        if (raw.empty()) {
            return {};
        }
        return resolve(raw);
    }

    void stack_trace_utils_t::clear_symbol_cache() noexcept {
        tls_cache.entries.clear();
        tls_cache.hits = 0;
        tls_cache.misses = 0;
        tls_cache.evictions = 0;
    }

    stack_trace_utils_t::cache_stats_t stack_trace_utils_t::get_cache_stats() noexcept {
        cache_stats_t stats;
        stats.hits = tls_cache.hits;
        stats.misses = tls_cache.misses;
        stats.evictions = tls_cache.evictions;
        return stats;
    }

#else  // !ERROR_SYSTEM_ENABLE_STACKTRACE

    std::vector<void*> stack_trace_utils_t::capture(int /*skip_frames*/, int /*max_frames*/) noexcept {
        return {};
    }

    std::vector<std::string> stack_trace_utils_t::resolve(const void* const* /*frames*/, size_t /*count*/) noexcept {
        return {};
    }

    std::vector<std::string> stack_trace_utils_t::resolve(const std::vector<void*>& /*frames*/) noexcept {
        return {};
    }

    std::vector<std::string> stack_trace_utils_t::generate(int /*skip_frames*/, int /*max_frames*/) noexcept {
        return {};
    }

    void stack_trace_utils_t::clear_symbol_cache() noexcept {}

    stack_trace_utils_t::cache_stats_t stack_trace_utils_t::get_cache_stats() noexcept {
        return {};
    }

#endif  // ERROR_SYSTEM_ENABLE_STACKTRACE

}  // namespace error_system::utils
