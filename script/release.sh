#!/bin/bash
set -e

# 版本发布脚本
# 用法: ./script/release.sh <version>  (例如: v4.4.0)
# 从 CHANGELOG.md 提取对应版本描述作为 tag message 与 Release Notes

if [ -z "$1" ]; then
    echo "用法: $0 <version>  (例如: v4.4.0)"
    exit 1
fi

VERSION="$1"
VERSION_NUM="${VERSION#v}"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="$(dirname "$SCRIPT_DIR")"

cd "$SOURCE_DIR"

echo "=== 发布版本 $VERSION ==="

# 1. 检查工作区
if [ -n "$(git status --porcelain)" ]; then
    echo "错误: 工作区不干净，请先提交所有改动"
    git status --short
    exit 1
fi

# 2. 检查当前分支
BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [ "$BRANCH" != "main" ] && [ "$BRANCH" != "master" ]; then
    echo "错误: 请在 main/master 分支发布，当前分支: $BRANCH"
    exit 1
fi

# 3. 检查 tag 是否已存在
if git rev-parse "$VERSION" >/dev/null 2>&1; then
    echo "错误: tag $VERSION 已存在"
    exit 1
fi

# 4. 检查 CHANGELOG.md 是否存在
if [ ! -f "CHANGELOG.md" ]; then
    echo "错误: CHANGELOG.md 不存在，请先添加版本 $VERSION 的变更记录"
    exit 1
fi

# 5. 从 CHANGELOG.md 提取对应版本描述
#    匹配 "## [v4.4.0]" 到下一个 "## [" 之间的内容
CHANGELOG_CONTENT=$(awk -v ver="$VERSION" '
    $0 ~ "^## \\[" ver "\\]" { found=1; next }
    /^## \[/ && found { exit }
    found { print }
' CHANGELOG.md)

if [ -z "$CHANGELOG_CONTENT" ]; then
    echo "错误: CHANGELOG.md 中未找到版本 $VERSION 的变更记录"
    echo "请确认 CHANGELOG.md 中存在 \"## [$VERSION]\" 条目"
    exit 1
fi

# 6. 生成 Release Notes 文件
RELEASE_NOTES="RELEASE_NOTES_${VERSION_NUM}.md"
{
    echo "# Release $VERSION"
    echo ""
    echo "$CHANGELOG_CONTENT"
} > "$RELEASE_NOTES"

echo "已生成 Release Notes: $RELEASE_NOTES"

# 7. 确认发布
echo ""
echo "即将发布版本 $VERSION (基于分支 $BRANCH)"
echo ""
echo "--- Release Notes 预览 ---"
cat "$RELEASE_NOTES"
echo "--------------------------"
echo ""
read -p "确认发布? (y/N) " CONFIRM
if [ "$CONFIRM" != "y" ] && [ "$CONFIRM" != "Y" ]; then
    echo "已取消"
    rm -f "$RELEASE_NOTES"
    exit 0
fi

# 8. Debug 构建 + 测试
echo ""
echo "=== [1/4] Debug 构建 + 测试 ==="
./script/build_debug.sh
echo "--- 运行 Debug 测试 ---"
(cd build-debug && ctest --output-on-failure)

# 9. Release 构建 + 测试
echo ""
echo "=== [2/4] Release 构建 + 测试 ==="
./script/build_release.sh
echo "--- 运行 Release 测试 ---"
(cd build-release && ctest --output-on-failure)

# 10. 打 tag（使用 CHANGELOG 内容作为 annotation message）
echo ""
echo "=== [3/4] 打 tag ==="
git tag -a "$VERSION" -F "$RELEASE_NOTES"

# 11. 推送 tag
echo ""
echo "=== [4/4] 推送 tag ==="
git push origin "$VERSION"

echo ""
echo "=== 发布完成: $VERSION ==="
echo "Release Notes 已保存至 $RELEASE_NOTES"
echo "GitHub Release 页面可基于此 tag 创建 Release，内容粘贴 $RELEASE_NOTES"
