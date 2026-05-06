#!/bin/bash
set -e  # 遇到任何错误立即退出

# 获取脚本所在目录的绝对路径（项目根目录）
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BIN_DIR="${SCRIPT_DIR}/bin"
EXECUTABLE="${BIN_DIR}/ChatServer"

echo "项目根目录: ${SCRIPT_DIR}"

# 1. 清理并创建 build 目录
if [ -d "${BUILD_DIR}" ]; then
    echo "清理 build 目录..."
    rm -rf "${BUILD_DIR:?}"/*
else
    echo "创建 build 目录..."
    mkdir -p "${BUILD_DIR}"
fi

# 2. 进入 build 目录并执行 cmake
cd "${BUILD_DIR}"
echo "运行 cmake 配置..."
cmake ..

# 3. 编译
echo "开始编译..."
make -j$(nproc)  # 使用多核加速

# 4. 检查可执行文件是否生成
if [ ! -f "${EXECUTABLE}" ]; then
    echo "错误: 编译后未找到可执行文件 ${EXECUTABLE}"
    exit 1
fi

# 5. 编译完成提示，不运行
echo "编译成功，可执行文件位于 ${EXECUTABLE}"