#pragma once
#include "error_system/utils/string_utils.h"

namespace error_system::utils {

    /**
     * @brief 将字符串解析为数字
     * @details 使用 std::from_chars 进行高效解析
     * @tparam T 目标数字类型
     * @param string 输入字符串
     * @return std::optional<T> 解析后的数字，失败返回 nullopt
     */
    template <typename T>
    inline std::optional<T> string_utils_t::parse_number(std::string_view string) noexcept {
        T value{};
        std::string str(string);
        const char* start = str.data();
        const char* end = start + str.size();

        if constexpr (std::is_integral_v<T>) {
            auto [ptr, ec] = std::from_chars(start, end, value);
            if (ec == std::errc{}) {
                return value;
            }
        } else if constexpr (std::is_floating_point_v<T>) {
            char* end_ptr = nullptr;
            auto result = std::strtod(start, &end_ptr);
            if (end_ptr == end) {
                return static_cast<T>(result);
            }
        }
        return std::nullopt;
    }

}  // namespace error_system::utils
