# Qt BiliCacheMerge 编译指南

## 🚀 快速开始

### 一键编译
```bash
cd /home/abelxiaoxing/work/BiliCacheMerge/qt_src
./build.sh
```

### 快速运行
```bash
./run.sh
```

## 📋 脚本说明

### `build.sh` - 编译脚本
**功能**: 一键完成Qt版本编译的完整流程

**用法**:
```bash
./build.sh          # 执行完整编译
./build.sh clean    # 清理构建文件
./build.sh help     # 显示帮助信息
```

**编译流程**:
1. ✅ 检查编译依赖 (Qt6.9.2, CMake, G++)
2. ✅ 配置Qt环境变量
3. ✅ 检查并创建缺失的图标文件
4. ✅ 准备build目录
5. ✅ CMake配置
6. ✅ 多线程编译
7. ✅ 验证编译结果

### `run.sh` - 运行脚本
**功能**: 快速启动编译好的应用程序

**用法**:
```bash
./run.sh                    # 启动应用
./run.sh /path/to/videos    # 启动并指定视频目录
./run.sh help              # 显示帮助信息
```

## 📁 输出文件

编译成功后，在 `build/` 目录下会生成：

```
build/
├── bin/
│   └── BiliCacheMerge     # 主程序 (374KB)
├── simple_test           # 简单测试程序
└── test_scanner_cli      # 扫描器测试程序
```

## ⚙️ 环境要求

### 必需依赖
- **Qt 6.9.2** - 安装在 `/home/abelxiaoxing/Qt/6.9.2/gcc_64`
- **CMake 3.16+** - 构建系统
- **G++ 7.0+** - C++17编译器
- **Python 3** - 用于创建占位图标

### 系统库依赖
脚本会自动设置以下环境变量：
```bash
export PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/bin:$PATH"
export CMAKE_PREFIX_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64"
export LD_LIBRARY_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/lib:$LD_LIBRARY_PATH"
```

## 🔧 手动编译步骤

如果脚本出现问题，可以手动执行以下步骤：

```bash
# 1. 进入qt_src目录
cd /home/abelxiaoxing/work/BiliCacheMerge/qt_src

# 2. 设置环境变量
export PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/bin:$PATH"
export CMAKE_PREFIX_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64"

# 3. 创建build目录
rm -rf build && mkdir build && cd build

# 4. CMake配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# 5. 编译
make -j$(nproc)

# 6. 运行
cd ..
export LD_LIBRARY_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/lib:$LD_LIBRARY_PATH"
./build/bin/BiliCacheMerge
```

## 🐛 常见问题

### 1. 图标文件缺失
**问题**: 编译时报错缺少PNG图标文件
**解决**: 脚本会自动创建占位图标，无需手动处理

### 2. Qt6 API兼容性
**问题**: `setCapStyle` 和 `setJoinStyle` 方法不存在
**解决**: 脚本已修复此问题，改为在QPen对象上设置

### 3. 库依赖问题
**问题**: 运行时找不到Qt库
**解决**: 确保设置了正确的 `LD_LIBRARY_PATH`

### 4. 权限问题
**问题**: 脚本无法执行
**解决**:
```bash
chmod +x build.sh run.sh
```

## 📊 编译输出示例

成功编译后你会看到类似输出：
```
[STEP] 检查编译依赖...
[SUCCESS] 依赖检查通过
[INFO]   - Qt: 6.9.2
[INFO]   - CMake: 4.1.2
[INFO]   - G++: 14.2.0

[STEP] 设置Qt环境变量...
[SUCCESS] Qt环境配置完成

[STEP] 检查图标资源文件...
[SUCCESS] 所有图标文件已存在

[STEP] 准备构建目录...
[SUCCESS] 构建目录准备完成

[STEP] 执行CMake配置...
[SUCCESS] CMake配置成功

[STEP] 开始编译项目...
[INFO] 使用 8 个CPU核心进行编译
[SUCCESS] 编译完成

[STEP] 验证编译结果...
[SUCCESS] 可执行文件生成成功
[INFO]   - 文件路径: /home/abelxiaoxing/work/BiliCacheMerge/qt_src/build/bin/BiliCacheMerge
[INFO]   - 文件大小: 374 KB
[INFO]   - 文件类型: ELF 64-bit LSB pie executable, x86-64

[SUCCESS] 所有任务完成！Qt版本BiliCacheMerge编译成功！
```

## 🎯 下一步

编译完成后，你可以：

1. **运行主程序**: `./run.sh`
2. **运行测试**: `./build/simple_test`
3. **开发调试**: 使用Qt Creator打开项目
4. **打包部署**: 参考Qt部署文档

---

**作者**: 哈雷酱大小姐
**更新时间**: 2025-01-21
**版本**: Qt v2.0