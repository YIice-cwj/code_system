#!/usr/bin/env python3
import os
import re

def get_project_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def generate_category_traits(root_dir, category):
    src_dir = os.path.join(root_dir, f"include/error_system/{category}")
    out_dir = os.path.join(root_dir, f"include/error_system/traits/{category}")

    if not os.path.isdir(src_dir):
        print(f"⚠️ 目录不存在: {src_dir}，跳过。")
        return

    os.makedirs(out_dir, exist_ok=True)
    print("=====================================")
    print(f"🔍 开始扫描分类: {category}")
    print("=====================================")

    for filename in sorted(os.listdir(src_dir)):
        if not filename.endswith(f"_{category}.h"):
            continue

        file_path = os.path.join(src_dir, filename)
        basename = filename.replace(f"_{category}.h", "")
        enum_name = f"{basename}_{category}_t"
        out_filename = f"{basename}_{category}_traits.h"
        out_file = os.path.join(out_dir, out_filename)

        print(f"⚙️  Processing {filename} -> {out_filename}")

        items = []
        with open(file_path, "r", encoding="utf-8") as f:
            in_enum = False
            for line in f:
                line = line.strip()
                if "enum class" in line:
                    in_enum = True
                elif in_enum and "};" in line:
                    in_enum = False
                elif in_enum and "=" in line:
                    # Extract the word before '='
                    match = re.search(r"([a-zA-Z0-9_]+)\s*=", line)
                    if match:
                        item = match.group(1)
                        if item != "_count":
                            items.append(item)

        if not items:
            print(f"   ⚠️ No enum items found in {file_path}")
            continue

        first_item = items[0]
        last_item = items[-1]

        # Generate traits content
        lines = []
        lines.append("#pragma once")
        lines.append(f'#include "error_system/{category}/{filename}"')
        lines.append(f'#include "error_system/traits/{category}_traits.h"')
        lines.append('#include "error_system/utils/string_utils.h"')
        lines.append("")
        lines.append("namespace error_system::traits {")
        lines.append("    template <>")
        lines.append(f"    struct {category}_traits<{category}::{enum_name}, void> {{")
        lines.append("")
        lines.append(f"        using {category}_t = {category}::{enum_name};")
        lines.append(f"        using underlying_t = std::underlying_type_t<{category}_t>;")
        lines.append("")
        lines.append("        /**")
        lines.append("         * @brief 将枚举值转换为整数")
        lines.append(f"         * @param {category} 枚举值")
        lines.append("         * @return 对应的整数值")
        lines.append("         */")
        lines.append(f"        static constexpr underlying_t to_int({category}_t {category}) noexcept {{")
        lines.append(f"            return static_cast<underlying_t>({category});")
        lines.append("        }")
        lines.append("")
        lines.append("        /**")
        lines.append("         * @brief 校验整数是否为有效的枚举值")
        lines.append("         * @param value 要校验的整数")
        lines.append("         * @return 如果是有效的枚举值，返回 true；否则返回 false")
        lines.append("         */")
        lines.append("        static constexpr bool is_valid(underlying_t value) noexcept {")
        lines.append(f"            return value >= to_int({category}_t::{first_item}) && value <= to_int({category}_t::{last_item});")
        lines.append("        }")
        lines.append("")
        lines.append("        /**")
        lines.append("         * @brief 从整数转换为枚举值")
        lines.append("         * @param value 要转换的整数")
        lines.append(f"         * @return 对应的枚举值，如果无效则返回 {category}_t::{first_item}")
        lines.append("         */")
        lines.append(f"        static constexpr {category}_t from_int(underlying_t value) noexcept {{")
        lines.append("            if (!is_valid(value)) {")
        lines.append(f"                return {category}_t::{first_item};")
        lines.append("            }")
        lines.append(f"            return static_cast<{category}_t>(value);")
        lines.append("        }")
        lines.append("")
        lines.append("        /**")
        lines.append("         * @brief 将枚举值转换为字符串")
        lines.append(f"         * @param {category} 枚举值")
        lines.append("         * @return 对应的字符串")
        lines.append("         */")
        lines.append(f"        static constexpr const char* to_string({category}_t {category}) noexcept {{")
        lines.append(f"            switch ({category}) {{")

        for item in items:
            lines.append(f"                case {category}_t::{item}:")
            lines.append(f'                    return "{item}";')

        lines.append("                default:")
        lines.append('                    return "none";')
        lines.append("            }")
        lines.append("        }")
        lines.append("")
        lines.append("        /**")
        lines.append("         * @brief 从字符串转换为枚举值")
        lines.append("         * @param string 要转换的字符串")
        lines.append(f"         * @return 对应的枚举值，如果无效则返回 {category}_t::{first_item}")
        lines.append("         */")
        lines.append(f"        static constexpr {category}_t from_string(const char* string) noexcept {{")
        lines.append("            switch (utils::string_utils_t::hash(string)) {")

        for item in items:
            lines.append(f'                case utils::string_utils_t::hash("{item}"):')
            lines.append(f"                    return {category}_t::{item};")

        lines.append("                default:")
        lines.append(f"                    return {category}_t::{first_item};")
        lines.append("            }")
        lines.append("        }")
        lines.append("    };")
        lines.append("}  // namespace error_system::traits")
        lines.append("")

        with open(out_file, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))

def main():
    root_dir = get_project_root()
    generate_category_traits(root_dir, "subsystem")
    generate_category_traits(root_dir, "module")
    
    print("=====================================")
    print("✅ Done! 所有 traits 特化生成完毕！")

if __name__ == "__main__":
    main()
