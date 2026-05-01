#!/usr/bin/env python3
import os
import re

def get_project_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def extract_enum_items(file_path):
    """从头文件中提取枚举项"""
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
                # 匹配 '=' 之前的单词
                match = re.search(r"([a-zA-Z0-9_]+)\s*=", line)
                if match:
                    item = match.group(1)
                    if item != "_count":
                        items.append(item)
    return items

def extract_range(file_path):
    """从头文件注释中提取范围信息"""
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()
        match = re.search(r'范围：(0x[0-9A-Fa-f]+)\s*-\s*(0x[0-9A-Fa-f]+)', content)
        if match:
            return match.group(1), match.group(2)
    return None, None

def generate_category_traits(root_dir, category):
    """为指定分类（module/subsystem）生成所有单独的 traits 文件"""
    src_dir = os.path.join(root_dir, f"include/error_system/{category}")
    out_dir = os.path.join(root_dir, f"include/error_system/traits/{category}")

    if not os.path.isdir(src_dir):
        print(f"⚠️ 目录不存在: {src_dir}，跳过。")
        return []

    os.makedirs(out_dir, exist_ok=True)
    generated_files = []

    for filename in sorted(os.listdir(src_dir)):
        if not filename.endswith(f"_{category}.h"):
            continue

        file_path = os.path.join(src_dir, filename)
        basename = filename.replace(f"_{category}.h", "")
        enum_name = f"{basename}_{category}_t"
        out_filename = f"{basename}_{category}_traits.h"
        out_file = os.path.join(out_dir, out_filename)

        items = extract_enum_items(file_path)
        if not items:
            continue

        first_item = items[0]
        last_item = items[-1]

        lines = [
            "#pragma once",
            "",
            "/**",
            f" * @file {out_filename}",
            f" * @brief {category.capitalize()} {basename} traits specialization",
            f" * @details 自动生成的 {category} {basename} traits 特化，用于提供枚举值的元数据和转换功能",
            " * @author antigravity",
            " * @version 1.0.0",
            " * @copyright Copyright (c) 2026",
            " */",
            "",
            f'#include "error_system/{category}/{filename}"',
            f'#include "error_system/traits/{category}_traits.h"',
            '#include "error_system/utils/string_utils.h"',
            "",
            "namespace error_system::traits {",
            "",
            "    /**",
            f"     * @brief {category}::{enum_name} 的 traits 特化",
            f"     * @details 提供 {enum_name} 与整数及字符串之间的转换和校验",
            "     */",
            "    template <>",
            f"    struct {category}_traits<{category}::{enum_name}, void> {{",
            "",
            "        /** @brief 关联的枚举类型 */",
            f"        using {category}_t = {category}::{enum_name};",
            "        /** @brief 枚举底层的整数类型 */",
            f"        using underlying_t = std::underlying_type_t<{category}_t>;",
            "",
            "        /**",
            "         * @brief 将枚举值转换为底层整数值",
            f"         * @param {category} 枚举实例",
            "         * @return underlying_t 对应的整数值",
            "         */",
            f"        static constexpr underlying_t to_int({category}_t {category}) noexcept {{",
            f"            return static_cast<underlying_t>({category});",
            "        }",
            "",
            "        /**",
            "         * @brief 校验整数值是否在枚举定义的有效范围内",
            "         * @param value 待校验的整数值",
            "         * @return bool 如果有效返回 true，否则返回 false",
            "         */",
            "        static constexpr bool is_valid(underlying_t value) noexcept {",
            f"            return value >= to_int({category}_t::{first_item}) && value <= to_int({category}_t::{last_item});",
            "        }",
            "",
            "        /**",
            "         * @brief 从底层整数值转换为枚举值",
            "         * @param value 整数值",
            f"         * @return {category}_t 对应的枚举值；如果值无效，返回 {category}_t::{first_item}",
            "         */",
            f"        static constexpr {category}_t from_int(underlying_t value) noexcept {{",
            "            if (!is_valid(value)) {",
            f"                return {category}_t::{first_item};",
            "            }",
            f"            return static_cast<{category}_t>(value);",
            "        }",
            "",
            "        /**",
            "         * @brief 将枚举值转换为对应的字符串名称",
            f"         * @param {category} 枚举实例",
            "         * @return const char* 对应的字符串名称，若无效返回 \"none\"",
            "         */",
            f"        static constexpr const char* to_string({category}_t {category}) noexcept {{",
            f"            switch ({category}) {{",
        ]

        for item in items:
            lines.append(f"                case {category}_t::{item}: return \"{item}\";")

        lines += [
            "                default: return \"none\";",
            "            }",
            "        }",
            "",
            "        /**",
            "         * @brief 从字符串名称转换为对应的枚举值",
            "         * @param string 字符串名称",
            f"         * @return {category}_t 对应的枚举值；如果无匹配，返回 {category}_t::{first_item}",
            "         */",
            f"        static constexpr {category}_t from_string(const char* string) noexcept {{",
            "            switch (utils::string_utils_t::hash(string)) {",
        ]

        for item in items:
            lines.append(f'                case utils::string_utils_t::hash("{item}"): return {category}_t::{item};')

        lines += [
            "                default:",
            f"                    return {category}_t::{first_item};",
            "            }",
            "        }",
            "    };",
            "}  // namespace error_system::traits"
        ]

        with open(out_file, "w", encoding="utf-8") as f:
            f.write("\n".join(lines))
        generated_files.append(out_filename)
        
    return generated_files

def generate_umbrella_headers(root_dir):
    """生成 subsystem_traits.h 和 module_traits.h 聚合文件"""
    for category in ["subsystem", "module"]:
        traits_dir = os.path.join(root_dir, f"include/error_system/traits/{category}")
        out_file = os.path.join(root_dir, f"include/error_system/traits/{category}/{category}_traits.h")
        
        lines = [
            "#pragma once",
            "",
            "/**",
            f" * @file {category}_traits.h",
            f" * @brief {category.capitalize()} Traits 统一入口",
            " */",
            "",
            "// IWYU pragma: begin_exports"
        ]
        
        for fname in sorted(os.listdir(traits_dir)):
            if fname.endswith(f"_{category}_traits.h") and fname != f"{category}_traits.h":
                lines.append(f'#include "error_system/traits/{category}/{fname}"')
                
        lines.append("// IWYU pragma: end_exports")
        
        with open(out_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        print(f"  📄 Generated {out_file}")

def generate_dispatchers(root_dir):
    """生成 module_dispatcher.h 和 subsystem_dispatcher.h"""
    for category in ["module", "subsystem"]:
        traits_dir = os.path.join(root_dir, f"include/error_system/traits/{category}")
        out_file = os.path.join(root_dir, f"include/error_system/traits/{category}_dispatcher.h")

        header = [
            "#pragma once",
            "",
            "/**",
            f" * @file {category}_dispatcher.h",
            f" * @brief {category.capitalize()} dispatcher for metadata resolution",
            f" * @details 自动生成的 {category} 分发器，用于根据数值范围或类型字符串解析元数据",
            " * @author antigravity",
            " * @version 1.0.0",
            " * @copyright Copyright (c) 2026",
            " */",
            "",
            "#include <cstdint>",
            f'#include "error_system/{category}/{category}.h"',
            f'#include "error_system/traits/{category}/{category}_traits.h"',
            '#include "error_system/utils/string_utils.h"',
            "",
            "namespace error_system::traits {",
            "",
            "    /**",
            f"     * @brief 根据数值解析 {category} 名称",
            f"     * @param value {category} 的整数值",
            f"     * @return const char* 对应的名称字符串，若无匹配返回 \"none\"",
            "     */",
            f"    static constexpr const char* resolve_{category}(uint16_t value) noexcept {{",
        ]

        from_string = [
            "",
            "    /**",
            f"     * @brief 根据类型和名称解析 {category} 枚举值",
            f"     * @param type {category} 的分类名称",
            f"     * @param name {category} 的具体名称",
            f"     * @return uint16_t 对应的枚举整数值，若无匹配返回 0",
            "     */",
            f"    static constexpr uint16_t {category}_from_string(const char* type, const char* name) noexcept {{",
            "        switch (utils::string_utils_t::hash(type)) {"
        ]

        for fname in sorted(os.listdir(traits_dir)):
            if fname.endswith(f"_{category}_traits.h") and fname != f"{category}_traits.h":
                base_name = fname.replace("_traits.h", "")
                type_name = base_name.replace(f"_{category}", "")
                enum_type = f"{category}::{base_name}_t"
                
                # resolve
                header.append(f"        if (traits::{category}_traits<{enum_type}>::is_valid(value))")
                header.append(f"            return traits::{category}_traits<{enum_type}>::to_string(static_cast<{enum_type}>(value));")

                # from_string
                from_string.append(f'            case utils::string_utils_t::hash("{type_name}"):')
                from_string.append(f"                return static_cast<uint16_t>(traits::{category}_traits<{enum_type}>::from_string(name));")

        header.append('        return "none";')
        header.append("    }")

        from_string.append("            default: return 0;")
        from_string.append("        }")
        from_string.append("    }")
        from_string.append("")
        from_string.append(f"}}  // namespace error_system::traits")

        with open(out_file, 'w', encoding='utf-8') as f:
            f.write('\n'.join(header + from_string))
        print(f"  📄 Generated {out_file}")

def main():
    root_dir = get_project_root()
    
    print("🔨 Starting Error System Traits Generation...")
    
    # 1. 生成各模块/子系统的单独 traits 文件
    print("  Step 1: Generating individual trait files...")
    generate_category_traits(root_dir, "module")
    generate_category_traits(root_dir, "subsystem")
    
    # 2. 生成聚合头文件
    print("  Step 2: Generating aggregate umbrella headers...")
    generate_umbrella_headers(root_dir)
    
    # 3. 生成分发器头文件
    print("  Step 3: Generating dispatcher headers...")
    generate_dispatchers(root_dir)
    
    print("\n✅ All done! Error system traits are up to date.")

if __name__ == "__main__":
    main()
