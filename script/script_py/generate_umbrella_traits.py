#!/usr/bin/env python3
import os

def get_project_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))

def generate_umbrella_traits(root_dir, domains):
    """生成 traits/subsystem/subsystem_traits.h 和 traits/module/module_traits.h 聚合头文件"""

    subsystem_traits_file = os.path.join(root_dir, "include/error_system/traits/subsystem/subsystem_traits.h")
    module_traits_file    = os.path.join(root_dir, "include/error_system/traits/module/module_traits.h")

    # subsystem_traits.h
    sub_lines = [
        "#pragma once",
        "",
        "/**",
        " * @file subsystem_traits.h",
        " * @brief 子系统 Traits 统一入口",
        " * @details 统一包含所有子系统 Traits 特化，方便一次性引入",
        " * @author yiice",
        " * @version 1.0.0",
        " * @date 2026-04-27",
        " * @copyright Copyright (c) 2026",
        " */",
        "",
        "// IWYU pragma: begin_exports",
    ]
    for d in sorted(domains):
        sub_lines.append(f'#include "error_system/traits/subsystem/{d}_subsystem_traits.h"')
    sub_lines += ["// IWYU pragma: end_exports", ""]

    with open(subsystem_traits_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(sub_lines))
    print(f"  📄 Generated {subsystem_traits_file}")

    # module_traits.h
    mod_lines = [
        "#pragma once",
        "",
        "/**",
        " * @file module_traits.h",
        " * @brief 模块 Traits 统一入口",
        " * @details 统一包含所有模块 Traits 特化，方便一次性引入",
        " * @author yiice",
        " * @version 1.0.0",
        " * @date 2026-04-27",
        " * @copyright Copyright (c) 2026",
        " */",
        "",
        "// IWYU pragma: begin_exports",
    ]
    # scan all *_module_traits.h files to catch common_module_traits etc.
    module_traits_dir = os.path.join(root_dir, "include/error_system/traits/module")
    for fname in sorted(os.listdir(module_traits_dir)):
        if fname.endswith("_module_traits.h") and fname != "module_traits.h":
            mod_lines.append(f'#include "error_system/traits/module/{fname}"')
    mod_lines += ["// IWYU pragma: end_exports", ""]

    with open(module_traits_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(mod_lines))
    print(f"  📄 Generated {module_traits_file}")

def main():
    root_dir = get_project_root()
    domain_file = os.path.join(root_dir, "include/error_system/domain/system_domain.h")
    
    # Extract domains
    domains = []
    with open(domain_file, 'r', encoding='utf-8') as f:
        in_enum = False
        for line in f:
            if "enum class system_domain_t" in line:
                in_enum = True
            elif in_enum and "}" in line:
                break
            elif in_enum:
                line = line.strip()
                if line and not line.startswith('//') and '=' in line:
                    domain = line.split('=')[0].strip()
                    if domain != "_count":
                        domains.append(domain)

    print("🔨 Generating umbrella trait headers...")
    generate_umbrella_traits(root_dir, domains)

if __name__ == "__main__":
    main()
