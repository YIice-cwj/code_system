#pragma once

#include "error_system/utils/log.h"

/**
 * @file bad_alloc_handler.h
 * @brief std::bad_alloc 统一报告工具
 * @details 项目内 noexcept 函数捕获 std::bad_alloc 后，处理逻辑高度一致：
 *          通过统一日志系统输出 "[module] func: std::bad_alloc"，
 *          再由调用方决定后续动作（设置标志、返回默认值等）。
 *          本工具消除 30+ 处重复的日志调用，统一格式与输出目标，符合 DRY 原则（规范 24.6）。
 * @author yiice
 * @version 1.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */
namespace error_system::utils {

    /**
     * @brief 报告 std::bad_alloc 发生
     * @details 通过 LOG_ERROR 输出 "[module] func: std::bad_alloc"。
     *          noexcept 安全，不会抛异常。调用方应在 catch (const std::bad_alloc&) 块内调用。
     * @param module 模块名（如 "error_context"、"plugin_registry"）
     * @param func 函数名（如 "make_error"、"enqueue_deferred_notification"）
     */
    inline void report_bad_alloc(const char* module, const char* func) noexcept {
        LOG_ERROR("[{}] {}: std::bad_alloc", module, func);
    }

}  // namespace error_system::utils
