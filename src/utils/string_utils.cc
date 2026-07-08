#include "error_system/utils/string_utils.h"

/**
 * @file string_utils.cc
 * @brief 字符串工具实现
 * @details 提供字符串替换、分割、合并、首尾空白去除、大小写转换等基础字符串操作能力。
 * @author yiice
 * @version 3.0.1
 * @date 2026-07-08
 * @copyright Copyright (c) 2026
 */

#include <algorithm>
#include <cctype>

#include "error_system/utils/log.h"

namespace error_system::utils {
    namespace {
        /**
         * @brief 通用大小写转换实现
         * @details 预分配并逐字符应用转换函数，内存不足时返回空字符串
         * @param string 输入字符串视图
         * @param transform 字符转换函数（std::tolower / std::toupper）
         * @param fn_name 调用方函数名，用于日志上报
         * @return std::string 转换后的字符串
         */
        std::string transform_chars_(std::string_view string, int (*transform)(int),
                                     const char* fn_name) noexcept {
            std::string result{};
            try {
                result.resize(string.size());
            } catch (const std::bad_alloc&) {
                LOG_ERROR("[string_utils] {}: resize failed", fn_name);
                return {};
            }
            std::transform(string.begin(), string.end(), result.begin(),
                           [transform](unsigned char character) {
                               return static_cast<char>(transform(character));
                           });
            return result;
        }
    }  // namespace

    /**
     * @brief 替换字符串中所有的指定子串
     * @details 改造为单次扫描追加模式，将复杂度从原先的 O(N^2) 降为 O(N)
     */
    std::string string_utils_t::replace_all(std::string string, std::string_view from, std::string_view to) noexcept {
        if (from.empty() || string.empty()) {
            return string;
        }

        size_t start_pos = string.find(from);
        if (start_pos == std::string::npos) {
            return string;
        }

        std::string result{};
        try {
            if (to.size() > from.size()) {
                result.reserve(string.size() + (to.size() - from.size()) * 2);
            } else {
                result.reserve(string.size());
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[string_utils] replace_all: reserve failed");
        }

        size_t current_pos = 0;
        try {
            while (start_pos != std::string::npos) {
                result.append(string.data() + current_pos, start_pos - current_pos);
                result.append(to);
                current_pos = start_pos + from.length();
                start_pos = string.find(from, current_pos);
            }

            if (current_pos < string.size()) {
                result.append(string.data() + current_pos, string.size() - current_pos);
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[string_utils] replace_all: append failed (bad_alloc)");
        }

        return result;
    }

    /**
     * @brief 分割字符串视图，返回视图向量
     * @details 该函数不修改原始字符串，仅返回分割后的视图向量
     * @param string 输入字符串视图
     * @param delimiter 分隔符视图
     * @return std::vector<std::string_view> 分割后的视图向量
     */
    std::vector<std::string_view> string_utils_t::split(std::string_view string, std::string_view delimiter) noexcept {
        if (string.empty() || delimiter.empty()) {
            return {};
        }
        std::vector<std::string_view> result{};
        try {
            size_t start = 0;
            size_t end = string.find(delimiter);
            while (end != std::string_view::npos) {
                if (start != end) {
                    result.push_back(string.substr(start, end - start));
                }
                start = end + delimiter.length();
                end = string.find(delimiter, start);
            }
            if (start < string.length()) {
                result.push_back(string.substr(start));
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[string_utils] split: push_back failed (bad_alloc)");
        }
        return result;
    }

    /**
     * @brief 合并字符串视图向量
     * @details 该函数不修改原始视图向量，仅返回合并后的字符串视图
     * @param tokens 输入字符串视图向量
     * @param delimiter 分隔符视图
     * @return std::string 合并后的字符串
     */
    std::string string_utils_t::join(const std::vector<std::string_view>& tokens, std::string_view delimiter) noexcept {
        if (tokens.empty()) {
            return {};
        }
        size_t total_length = 0;
        for (const auto& token : tokens) {
            total_length += token.size();
        }
        total_length += delimiter.size() * (tokens.size() - 1);
        std::string result{};
        try {
            result.reserve(total_length);
            result.append(tokens[0]);
            for (size_t i = 1; i < tokens.size(); ++i) {
                result.append(delimiter);
                result.append(tokens[i]);
            }
        } catch (const std::bad_alloc&) {
            LOG_ERROR("[string_utils] join: append failed (bad_alloc)");
        }
        return result;
    }

    /**
     * @brief 移除字符串视图首尾的空白符
     * @details 该函数不修改原始字符串视图，仅返回移除空白符后的视图
     * @param string 输入字符串视图
     * @return std::string_view 移除空白符后的字符串视图
     */
    std::string_view string_utils_t::trim(std::string_view string) noexcept {
        constexpr std::string_view WHITESPACE = " \t\n\r\f\v";
        size_t first = string.find_first_not_of(WHITESPACE);
        if (first == std::string_view::npos) {
            return {};
        }
        size_t last = string.find_last_not_of(WHITESPACE);
        return string.substr(first, (last - first + 1));
    }

    /**
     * @brief 将字符串视图转换为小写
     * @details 返回转换后的字符串，内存不足时返回空字符串
     * @param string 输入字符串视图
     * @return std::string 小写的字符串
     */
    std::string string_utils_t::to_lower(std::string_view string) noexcept {
        return transform_chars_(string, std::tolower, "to_lower");
    }

    /**
     * @brief 将字符串视图转换为大写
     * @details 返回转换后的字符串，内存不足时返回空字符串
     * @param string 输入字符串视图
     * @return std::string 大写的字符串
     */
    std::string string_utils_t::to_upper(std::string_view string) noexcept {
        return transform_chars_(string, std::toupper, "to_upper");
    }

}  // namespace error_system::utils