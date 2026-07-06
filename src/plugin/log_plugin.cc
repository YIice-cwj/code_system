#include "error_system/plugin/log_plugin.h"

/**
 * @file log_plugin.cc
 * @brief 错误日志插件实现
 * @details 将 error_context_t 格式化后写入目标流，mutex 保护避免多线程交错。
 * @author yiice
 * @version 3.0.0
 * @date 2026-07-04
 * @copyright Copyright (c) 2026
 */

#include <cstdio>
#include <iostream>
#include <new>
#include <utility>

namespace error_system::plugin {

    log_plugin_t::log_plugin_t(std::string name,
                                core::error_level_t min_level,
                                format_t format,
                                std::ostream* stream) noexcept
        : name_(std::move(name)),
          min_level_(min_level),
          format_(format),
          stream_(stream != nullptr ? stream : &std::cerr) {}

    void log_plugin_t::on_error(const core::error_context_t& context) noexcept {
        try {
            std::string formatted = (format_ == format_t::json)
                                        ? context.to_json()
                                        : context.to_string();
            std::lock_guard<std::mutex> lock(mutex_);
            (*stream_) << formatted << '\n';
            stream_->flush();
        } catch (const std::bad_alloc&) {
            std::fprintf(stderr, "[log_plugin] on_error: std::bad_alloc\n");
        } catch (...) {
            std::fprintf(stderr, "[log_plugin] on_error: unknown exception\n");
        }
    }

}  // namespace error_system::plugin
