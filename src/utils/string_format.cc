#include "error_system/utils/string_format.h"

#include "error_system/utils/log.h"

/**
 * @file string_format.cc
 * @brief 字符串格式化引擎实现
 * @details 提供 std::format 风格的占位符替换功能，支持算术类型、指针、bool 及自定义 to_string 类型
 * @author yiice
 * @version 3.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

namespace error_system::utils {

    /**
     * @brief 追加字面量大括号
     * @details 遍历格式字符串，将非占位符部分追加到结果中
     *          支持 {{ 和 }} 转义
     */
    void string_format_t::format_appender_t::append_literal_braces() noexcept {
        try {
            while (cursor < format.size()) {
                if (format[cursor] == '{') {
                    if (cursor + 1 < format.size() && format[cursor + 1] == '{') {
                        result.push_back('{');
                        cursor += 2;
                        continue;
                    }
                    break;
                }
                if (format[cursor] == '}') {
                    if (cursor + 1 < format.size() && format[cursor + 1] == '}') {
                        result.push_back('}');
                        cursor += 2;
                        continue;
                    }
                }
                result.push_back(format[cursor]);
                ++cursor;
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[string_format] append_literal_braces: bad_alloc");
        }
    }

}  // namespace error_system::utils
