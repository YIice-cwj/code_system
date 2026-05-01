#!/bin/bash

# 自动定位项目根目录（脚本所在的上一级目录）
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "🌐 启动 i18n 生成脚本..."
python3 "$PROJECT_ROOT/script/script_py/generate_i18n.py" "$@"
