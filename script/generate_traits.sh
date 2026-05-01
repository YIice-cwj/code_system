#!/bin/bash

# 自动定位项目根目录（脚本所在的上一级目录）
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# 抽象出一个通用的生成函数
# 参数 1: 分类名称（比如 subsystem 或 module）
generate_category_traits() {
    local category=$1
    local src_dir="$PROJECT_ROOT/include/error_system/$category"
    local out_dir="$PROJECT_ROOT/include/error_system/traits/$category"

    if [ ! -d "$src_dir" ]; then
        echo "⚠️ 目录不存在: $src_dir，跳过。"
        return
    fi

    mkdir -p "$out_dir"
    echo "====================================="
    echo "🔍 开始扫描分类: $category"
    echo "====================================="

    # 自动遍历目录下所有的 _${category}.h 文件
    for file in "$src_dir"/*_${category}.h; do
        if [ ! -f "$file" ]; then continue; fi

        filename=$(basename "$file")
        basename="${filename%_${category}.h}"
        enum_name="${basename}_${category}_t"
        out_file="$out_dir/${basename}_${category}_traits.h"
        
        echo "⚙️  Processing $filename -> $(basename "$out_file")"
        
        # 使用 awk 解析 enum 的项 (提取等号前面的非空单词)
        items=$(awk '
            /enum class/ { in_enum = 1; next }
            in_enum && /};/ { in_enum = 0; next }
            in_enum && /=/ { 
                match($0, /[a-zA-Z0-9_]+/)
                print substr($0, RSTART, RLENGTH)
            }
        ' "$file")
        
        if [ -z "$items" ]; then
            echo "   ⚠️ No enum items found in $file"
            continue
        fi

        # 获取第一个和最后一个枚举项，用于 is_valid
        first_item=$(echo "$items" | head -n 1)
        last_item=$(echo "$items" | tail -n 1)
        
        # 生成 traits 头文件
        cat <<OUT > "$out_file"
#pragma once
#include "error_system/${category}/${filename}"
#include "error_system/traits/${category}_traits.h"
#include "error_system/utils/string_utils.h"

namespace error_system::traits {
    template <>
    struct ${category}_traits<${category}::${enum_name}, void> {

        using ${category}_t = ${category}::${enum_name};
        using underlying_t = std::underlying_type_t<${category}_t>;

        /**
         * @brief 将枚举值转换为整数
         * @param ${category} 枚举值
         * @return 对应的整数值
         */
        static constexpr underlying_t to_int(${category}_t ${category}) noexcept {
            return static_cast<underlying_t>(${category});
        }

        /**
         * @brief 校验整数是否为有效的枚举值
         * @param value 要校验的整数
         * @return 如果是有效的枚举值，返回 true；否则返回 false
         */
        static constexpr bool is_valid(underlying_t value) noexcept {
            return value >= to_int(${category}_t::${first_item}) && value <= to_int(${category}_t::${last_item});
        }

        /**
         * @brief 从整数转换为枚举值
         * @param value 要转换的整数
         * @return 对应的枚举值，如果无效则返回 ${category}_t::${first_item}
         */
        static constexpr ${category}_t from_int(underlying_t value) noexcept {
            if (!is_valid(value)) {
                return ${category}_t::${first_item};
            }
            return static_cast<${category}_t>(value);
        }

        /**
         * @brief 将枚举值转换为字符串
         * @param ${category} 枚举值
         * @return 对应的字符串
         */
        static constexpr const char* to_string(${category}_t ${category}) noexcept {
            switch (${category}) {
OUT

        # 生成 to_string 的 case
        for item in $items; do
            cat <<OUT >> "$out_file"
                case ${category}_t::${item}:
                    return "${item}";
OUT
        done

        # 闭合 to_string 并开始 from_string
        cat <<OUT >> "$out_file"
                default:
                    return "unknown";
            }
        }

        /**
         * @brief 从字符串转换为枚举值
         * @param string 要转换的字符串
         * @return 对应的枚举值，如果无效则返回 ${category}_t::${first_item}
         */
        static constexpr ${category}_t from_string(const char* string) noexcept {
            switch (utils::string_utils_t::hash(string)) {
OUT

        # 生成 from_string 的 case
        for item in $items; do
            cat <<OUT >> "$out_file"
                case utils::string_utils_t::hash("${item}"):
                    return ${category}_t::${item};
OUT
        done

        # 闭合 from_string 和 struct
        cat <<OUT >> "$out_file"
                default:
                    return ${category}_t::${first_item};
            }
        }
    };
}  // namespace error_system::traits
OUT

    done
}

# 执行生成逻辑
generate_category_traits "subsystem"
generate_category_traits "module"

echo "====================================="
echo "✅ Done! 所有 traits 特化生成完毕！"
