#!/bin/bash

# GUI测试脚本
echo "=== Qt B站缓存合并工具 GUI测试 ==="
echo "测试目录: /home/abelxiaoxing/work/BiliCacheMerge/test_data"
echo ""

cd /home/abelxiaoxing/work/BiliCacheMerge/qt_src/build

echo "启动GUI应用程序..."
echo "使用说明："
echo "1. 应用程序会自动设置测试目录"
echo "2. 等待1秒后自动开始扫描"
echo "3. 查看日志输出确认扫描结果"
echo "4. 点击'继续'按钮开始合并操作"
echo "5. 观察进度条和合并结果"
echo ""

# 启动GUI应用程序
./bin/BiliCacheMerge /home/abelxiaoxing/work/BiliCacheMerge/test_data

echo ""
echo "测试完成！"