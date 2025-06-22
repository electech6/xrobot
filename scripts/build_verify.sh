#!/bin/bash
# XRobot 编译验证脚本
# 用于验证ESP32平台编译是否成功

set -e

echo "=== XRobot 编译验证开始 ==="
echo "当前分支: $(git branch --show-current)"
echo "提交哈希: $(git rev-parse --short HEAD)"
echo ""

# 检查ESP-IDF环境
if [ -z "$IDF_PATH" ]; then
    echo "错误: ESP-IDF环境未设置，请先运行 'source $HOME/esp/esp-idf/export.sh'"
    exit 1
fi

echo "ESP-IDF路径: $IDF_PATH"
echo "ESP-IDF版本: $(idf.py --version)"
echo ""



# 执行编译
echo "开始编译..."
idf.py build

echo ""
echo "=== 编译验证成功 ==="
echo "构建时间: $(date)"