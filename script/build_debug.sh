#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOURCE_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${SOURCE_DIR}/build-debug"

echo "=== Debug 一键编译 ==="
echo "构建目录: ${BUILD_DIR}"

cmake -S "${SOURCE_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DERROR_SYSTEM_ENABLE_ASAN=ON \
    -DERROR_SYSTEM_ENABLE_UBSAN=ON \
    -DERROR_SYSTEM_ENABLE_STACKTRACE=ON \
    -DERROR_SYSTEM_ENABLE_VALIDATION=ON \
    -DERROR_SYSTEM_ENABLE_LOCATION=ON \
    -DERROR_SYSTEM_BUILD_TESTS=ON \
    -DERROR_SYSTEM_BUILD_EXAMPLES=ON \
    -DERROR_SYSTEM_BUILD_PERF_TESTS=ON

cmake --build "${BUILD_DIR}" --parallel $(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "=== Debug 一键编译 + 测试完成 ==="
