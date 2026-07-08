#include "error_system/plugin/log_plugin.h"

/**
 * @file log_plugin.cc
 * @brief 错误日志插件实现
 * @details 将 error_context_t 格式化后写入目标流，mutex 保护避免多线程交错。
 *          Lean 路径（on_code）仅输出 raw code，零字符串分配。
 * @author yiice
 * @version 4.4.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <iostream>
#include <new>
#include <utility>

#include "error_system/utils/bad_alloc_handler.h"
#include "error_system/utils/log.h"

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
            utils::report_bad_alloc("log_plugin", "on_error");
        } catch (const std::exception& e) {
            LOG_ERROR("[log_plugin] on_error: {}", e.what());
        }
    }

    void log_plugin_t::on_code(core::error_code_t code) noexcept {
        try {
            std::lock_guard<std::mutex> lock(mutex_);
            (*stream_) << "[ERR: " << code.get_code() << "]\n";
            stream_->flush();
        } catch (const std::exception& e) {
            LOG_ERROR("[log_plugin] on_code: {}", e.what());
        }
    }

}  // namespace error_system::plugin
