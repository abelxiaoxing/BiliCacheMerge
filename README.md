# BiliCacheMerge - B站缓存合并工具

<div align="center">

![BiliCacheMerge Logo](https://img.shields.io/badge/BiliCacheMerge-2.0.0-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)
![Qt](https://img.shields.io/badge/Qt-6.10.0-green.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

**专业的B站缓存文件合并工具，基于QT构建,支持多种客户端格式，使缓存视频能够正常播放** ✨

[📥 下载](#-下载) • [🚀 快速开始](#-快速开始) • [💡 功能特性](#-功能特性) • [📋 使用说明](#-使用说明) • [🛠️ 开发构建](#-开发构建) • [❓ 常见问题](#-常见问题)

</div>

---

## 🎯 项目简介

BiliCacheMerge 是一个基于 Qt6 和 C++ 开发的专业工具，专门用于合并手机端下载的B站缓存 m4s 文件，使其能够正常播放视频。该工具支持多种B站客户端的缓存格式，包括 Android、Windows、UWP 等不同平台的下载格式。

### ✨ 核心价值

- 🔧 **开箱即用** - 无需复杂配置，下载即用
- 🌍 **多平台支持** - Windows 和 Linux 双平台支持
- 📱 **多格式兼容** - 支持 Android、Windows、UWP 等多种客户端格式
- 🎬 **完整功能** - 视频合并 + 弹幕 + 字幕 + 封面，一站式解决方案
- 🏗️ **模块化设计** - 支持自定义模式，适应新版本客户端

---

## 📥 下载

### 🎁 Release 版本（推荐）

我们提供了预编译的 release 版本，开箱即用：

#### Windows 版本
- **文件**: `BiliCacheMerge-win-x86_64.7z`
- **大小**: 约 15 MB
- **包含**: 完整可执行文件 + FFmpeg + 所有依赖

#### Linux 版本
- **文件**: `BiliCacheMerge-linux-x86_64.7z`
- **大小**: 约 18 MB
- **包含**: 完整可执行文件 + FFmpeg + 所有依赖

#### 测试数据集
- **文件**: `test_data.7z`
- **用途**: 用于测试工具功能和熟悉操作流程
- **内容**: 包含不同客户端格式的示例缓存文件

### 📦 包含内容

每个 release 版本包含：
```
BiliCacheMerge/
├── BiliCacheMerge          # 主程序
├── ffmpeg/                 # FFmpeg 工具集
│   ├── ffmpeg             # 视频处理引擎
│   └── ffprobe            # 媒体信息分析工具
├── pattern/                # 客户端模式定义
│   ├── Android.pat        # Android 客户端模式
│   ├── Android_movie.pat  # Android 电影模式
│   ├── WIN10_official.pat # Windows 官方客户端模式
│   └── ...                # 其他模式文件
└── README.md               # 说明文档
```

---

## 🚀 快速开始

### 📋 系统要求

#### Windows 系统
- Windows 10/11 (64位)
- 无需额外安装 Qt 运行时库（已打包）

#### Linux 系统
- Ubuntu 18.04+ / CentOS 7+ / 其他主流发行版 (64位)
- 无需额外安装依赖（静态链接）

### 🎮 使用步骤

#### 1️⃣ 下载并解压
```bash
# Windows
# 下载 BiliCacheMerge-win-x86_64.7z 并解压到任意目录

# Linux
tar -xf BiliCacheMerge-linux-x86_64.7z
cd BiliCacheMerge
```

#### 2️⃣ 启动程序
```bash
# Windows
双击运行 BiliCacheMerge.exe

# Linux
./BiliCacheMerge
```

#### 3️⃣ 选择缓存目录
- 点击 **"选择目录"** 按钮
- 浏览并选择B站缓存所在的文件夹（通常是手机导出的 `Android/data/tv.danmaku.bili` 目录）

#### 4️⃣ 开始合并
- 程序会自动识别缓存格式
- 点击 **"开始合并"** 按钮
- 等待合并完成，输出文件将保存在原目录

---

## 💡 功能特性

### 🎯 核心功能

| 功能 | 说明 | 支持状态 |
|------|------|----------|
| 🎬 视频合并 | 将分离的视频和音频流合并为可播放的 MP4 文件 | ✅ 完全支持 |
| 📱 多格式支持 | 支持 Android、Windows、UWP 等多种客户端格式 | ✅ 完全支持 |
| 🎭 弹幕合并 | 将 XML 弹幕转换为 ASS 字幕并嵌入视频 | ✅ 完全支持 |
| 🖼️ 封面提取 | 自动提取并合并视频封面 | ✅ 完全支持 |
| 📝 字幕下载 | 自动下载并合并字幕文件 | ✅ 完全支持 |
| 🔧 自定义模式 | 支持用户自定义新客户端格式 | ✅ 完全支持 |
| ⚡ 批量处理 | 一次处理多个视频，支持分集合并 | ✅ 完全支持 |
| 📊 进度显示 | 实时显示处理进度和详细日志 | ✅ 完全支持 |
| 🔄 断点续传 | 支持合并中断后继续执行 | ✅ 完全支持 |
| 🐛 错误跳过 | 可配置跳过错误文件继续处理 | ✅ 完全支持 |

### 📱 支持的客户端格式

#### Android 客户端
- **标准模式** (`Android.pat`)
  - 文件结构：`video.m4s` + `audio.m4s` + `entry.json`
  - 特点：单个视频文件，无分组支持

- **电影模式** (`Android_movie.pat`)
  - 文件结构：按剧集组织
  - 特点：支持番剧/电影分集合并

#### Windows 客户端
- **官方客户端** (`WIN10_official.pat`)
  - 平台：Windows 10/11 UWP 官方客户端
  - 特点：官方标准格式

- **小洋葱客户端** (`UWP_xiaoyaocz_Ver3.pat`)
  - 平台：UWP 版小洋葱客户端
  - 特点：支持视频分组（合集）

#### 其他格式
- **Bilili 工具** (`Bilili_cmdtool.pat`)
  - 第三方下载工具格式

- **BLV 格式**（PC客户端）
  - 支持 PC 客户端的 BLV 格式
  - 支持分段 BLV 文件自动合并

---

## 📋 使用说明

### 🖥️ 界面介绍

主界面包含以下区域：

```
┌─────────────────────────────────────────┐
│  📂 B站缓存合并工具 v2.0.0               │
├─────────────────────────────────────────┤
│  📁 选择缓存目录: [目录路径____] [浏览]   │
│  📊 状态: ✅ 已就绪                      │
├─────────────────────────────────────────┤
│  ⚙️ 合并选项:                            │
│  ☑️ 合并弹幕 (ASS格式)                   │
│  ☑️ 提取封面                            │
│  ☑️ 下载字幕                            │
│  ☑️ 跳过错误文件                         │
├─────────────────────────────────────────┤
│  🚀 [开始合并]  ⏸️ [暂停]  ⏹️ [停止]    │
│  ████████░░░░░░░░░░  80% (进度条)        │
├─────────────────────────────────────────┤
│  📜 日志输出:                           │
│  [14:30:15] 开始扫描目录...              │
│  [14:30:16] 找到 15 个视频文件            │
│  [14:30:17] 开始合并: 第1集.mp4           │
│  ...                                    │
└─────────────────────────────────────────┘
```

### 🎛️ 配置选项

#### 合并模式选择
- **智能模式**：自动识别客户端格式
- **指定模式**：手动选择特定的客户端格式

#### 文件处理选项
- **覆盖输出**：是否覆盖已存在的输出文件
- **单目录模式**：只扫描当前目录，不递归子目录
- **顺序合并**：按照文件名顺序合并分集

#### 高级选项
- **错误跳过**：遇到错误文件时跳过继续处理
- **日志级别**：控制日志输出的详细程度
- **临时目录**：设置中间文件存储位置

### 📁 目录结构示例

典型的B站缓存目录结构：

```
📁 tv.danmaku.bili/
├── 📁 视频1/
│   ├── 📄 entry.json          # 元数据文件
│   ├── 📹 video.m4s           # 视频流
│   ├── 🎵 audio.m4s           # 音频流
│   ├── 💬 danmaku.xml         # 弹幕文件
│   └── 🖼️ cover.jpg           # 封面图片
├── 📁 视频2/
│   ├── 📄 entry.json
│   ├── 📹 video.m4s
│   ├── 🎵 audio.m4s
│   └── 💬 danmaku.xml
└── 📁 合集/
    ├── 📁 第1集/
    │   ├── 📄 entry.json
    │   ├── 📹 video.m4s
    │   └── 🎵 audio.m4s
    ├── 📁 第2集/
    │   ├── 📄 entry.json
    │   ├── 📹 video.m4s
    │   └── 🎵 audio.m4s
    └── 📄 entry.json          # 合集元数据
```

### 🎯 输出结果

合并完成后，输出文件结构：

```
📁 输出目录/
├── 📹 第1集.mp4                # 合并后的视频（含弹幕、字幕、封面）
├── 📹 第2集.mp4
├── 📝 第1集.ass               # 独立弹幕字幕文件
├── 📝 第2集.ass
└── 📜 merge_log.txt           # 合并日志
```

---

## 🛠️ 开发构建

### 🔧 开发环境要求

- **Qt**: 6.10.0 或更高版本
- **C++**: C++17 标准
- **CMake**: 3.16 或更高版本
- **编译器**:
  - Windows: MSVC 2019+ / MinGW-w64
  - Linux: GCC 8+ / Clang 10+

### 📦 依赖库

- **Qt6 Core**: 核心功能
- **Qt6 Widgets**: GUI界面
- **Qt6 Concurrent**: 多线程支持
- **Qt6 Network**: 网络功能（下载字幕、封面）

### 🏗️ 编译步骤

#### Windows (MSVC)

```powershell
# 1. 克隆项目
git clone https://github.com/your-repo/BiliCacheMerge.git
cd BiliCacheMerge

# 2. 创建构建目录
mkdir build
cd build

# 3. 配置 CMake
cmake .. -G "Visual Studio 16 2019" -A x64

# 4. 编译
cmake --build . --config Release

# 5. 打包
cd release
# 复制必要文件到 release 目录
```

#### Windows (MinGW)

```powershell
# 1. 安装 Qt6 (mingw_64 版本)
# 2. 配置 CMake
cmake .. -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.10.0/mingw_64

# 3. 编译
cmake --build . --config Release
```

#### Linux (GCC)

```bash
# 1. 安装依赖
sudo apt-get update
sudo apt-get install qt6-base-dev cmake build-essential

# 2. 克隆并编译
git clone https://github.com/your-repo/BiliCacheMerge.git
cd BiliCacheMerge
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 3. 安装
sudo make install
```

### 🏛️ 项目结构

```
BiliCacheMerge/
├── CMakeLists.txt           # CMake 构建配置
├── README.md                # 说明文档
├── src/                     # 源代码
│   ├── main.cpp             # 主函数入口
│   ├── application/         # GUI 应用层
│   │   ├── MainWindow.h/.cpp      # 主窗口
│   │   ├── ConfigDialog.h/.cpp    # 配置对话框
│   │   ├── HelpDialog.h/.cpp      # 帮助对话框
│   │   ├── LogViewer.h/.cpp       # 日志查看器
│   │   └── PatternBuilderDialog.h/.cpp # 模式创建向导
│   └── core/                # 核心功能层
│       ├── ConfigManager.h/.cpp   # 配置管理
│       ├── FfmpegManager.h/.cpp   # FFmpeg 管理
│       ├── PatternManager.h/.cpp  # 模式管理
│       ├── FileScanner.h/.cpp     # 文件扫描
│       ├── DanmakuConverter.h/.cpp # 弹幕转换
│       ├── SubtitleDownloader.h/.cpp # 字幕下载
│       ├── CoverDownloader.h/.cpp  # 封面下载
│       ├── MergeThread.h/.cpp     # 合并线程
│       └── Utils.h/.cpp           # 工具函数
├── pattern/                 # 客户端模式定义
│   ├── Android.pat
│   ├── Android_movie.pat
│   ├── WIN10_official.pat
│   ├── UWP_xiaoyaocz_Ver3.pat
│   └── Bilili_cmdtool.pat
├── ffmpeg/                  # FFmpeg 工具集
│   ├── ffmpeg               # Linux 版本
│   ├── ffmpeg.exe           # Windows 版本
│   ├── ffprobe              # Linux 版本
│   ├── ffprobe.exe          # Windows 版本
│   └── README.md            # FFmpeg 说明
├── resources/               # 资源文件
│   └── app.rc              # Windows 图标
├── build/                   # 构建输出目录
└── release/                 # 发布目录
```

### 🔄 持续集成

项目使用 GitHub Actions 进行 CI/CD：

- **Windows 构建**: MSVC 和 MinGW 双编译器
- **Linux 构建**: GCC 和 Clang 双编译器
- **自动化测试**: 单元测试 + 集成测试
- **打包发布**: 自动生成 release 包

---

## 🧪 测试

### 📦 使用测试数据集

我们提供了 `test_data.7z` 测试数据集，包含不同客户端格式的示例缓存文件：

```bash
# 1. 下载测试数据
# 下载 test_data.7z

# 2. 解压
7z x test_data.7z -o./test_data

# 3. 使用工具打开测试数据目录
./BiliCacheMerge ./test_data/Android_sample

# 4. 验证输出结果
ls -la output/
```

### ✅ 测试检查清单

- [ ] Android 标准格式合并正确
- [ ] Android 电影格式分集合并正确
- [ ] Windows 官方客户端格式兼容
- [ ] BLV 格式正确解析
- [ ] 弹幕转换为 ASS 字幕
- [ ] 封面提取并嵌入
- [ ] 字幕下载和合并
- [ ] 错误跳过功能正常
- [ ] 进度显示准确
- [ ] 日志记录完整

---

## ❓ 常见问题

### 🤔 FAQ

#### Q1: 程序无法启动，提示缺少 DLL？
**A**: 下载的 release 版本已经包含所有依赖，如果仍提示 DLL 错误，请：
1. 确保下载的是对应系统的版本（Windows x64 / Linux x64）
2. 以管理员权限运行（Windows）
3. 检查系统是否安装了 Visual C++ 2019+ 运行库

#### Q2: 提示 "未找到 FFmpeg"？
**A**: FFmpeg 已包含在 release 包中，如果仍报错：
1. 检查程序目录是否完整
2. 重新下载完整 release 包
3. Linux 版本确保文件有执行权限：`chmod +x BiliCacheMerge ffmpeg/ffmpeg`

#### Q3: 识别不到缓存文件？
**A**: 可能原因及解决方法：
1. **目录选择错误**：确保选择的是包含 `entry.json` 文件的目录
2. **格式不匹配**：尝试在设置中手动选择客户端格式
3. **文件损坏**：检查 `entry.json` 是否完整可用
4. **权限问题**：确保程序有读取目录的权限

#### Q4: 合并后视频无法播放？
**A**: 可能原因：
1. **原始文件损坏**：检查 `video.m4s` 和 `audio.m4s` 是否完整
2. **编码不兼容**：程序会自动处理大多数编码问题
3. **文件权限**：确保输出文件有读取权限

#### Q5: 弹幕无法显示？
**A**:
1. 确认选择了 "合并弹幕" 选项
2. 检查 `danmaku.xml` 文件是否存在
3. 部分播放器需要安装字幕插件（如 PotPlayer）
4. 独立字幕文件（.ass）可手动加载

#### Q6: 支持哪些视频格式？
**A**: 输出格式为 MP4，支持的输入格式：
- **视频**: m4s (H.264/H.265), mp4, avi, mkv
- **音频**: m4s (AAC), mp3, aac, flac
- **容器**: MP4（输出）

#### Q7: 如何处理大量视频？
**A**:
1. 使用 "批量模式" 选择包含多个视频的目录
2. 启用 "错误跳过" 选项避免单个文件错误中断流程
3. 建议分批处理（每次 50-100 个视频）
4. 确保有足够的磁盘空间（输出文件约为原文件 1.2 倍）

#### Q8: Linux 下中文字符显示乱码？
**A**:
```bash
# 安装中文字体支持
sudo apt-get install fonts-noto-cjk

# 设置环境变量
export LC_ALL=zh_CN.UTF-8
export LANG=zh_CN.UTF-8

# 重新运行程序
./BiliCacheMerge
```

#### Q9: 如何自定义新的客户端格式？
**A**: 使用内置的 "搜索模式创建向导"：
1. 菜单 → 工具 → 搜索模式创建向导
2. 按向导提示选择缓存目录和设置规则
3. 测试模式匹配效果
4. 保存为 `.pat` 文件
5. 将文件放入 `pattern/` 目录

#### Q10: 程序崩溃或无响应？
**A**:
1. 检查日志文件（`logs/app.log`）
2. 尝试用测试数据集复现问题
3. 减少单次处理的文件数量
4. 确保系统有足够内存（建议 4GB+）
5. 在 Issues 页面提交 Bug 报告

### 🐛 问题反馈

如遇到其他问题，请通过以下方式反馈：

- **GitHub Issues**: [https://github.com/your-repo/BiliCacheMerge/issues](https://github.com/your-repo/BiliCacheMerge/issues)
- **提交信息时请包含**:
  - 操作系统版本（Windows 10/11 或 Linux 发行版）
  - 程序版本号
  - 详细的错误日志
  - 复现步骤
  - 缓存目录结构截图

---

## 📄 许可证

本项目采用 MIT 许可证，详情请参阅 [LICENSE](LICENSE) 文件。

---

## 🤝 贡献

欢迎贡献代码！请阅读 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详细指南。

### 贡献方式

- 🐛 报告 Bug
- 💡 提出新功能建议
- 📝 完善文档
- 🔧 提交代码修复
- 🌐 翻译文档
- 📢 推荐给朋友

### 致谢

感谢所有为本项目做出贡献的开发者和用户！ ❤️

---

## 📞 联系我们

- **项目主页**: [https://github.com/abelxiaoxing/BiliCacheMerge](https://github.com/abelxiaoxing/BiliCacheMerge)
- **问题反馈**: [GitHub Issues](https://github.com/your-repo/BiliCacheMerge/issues)
- **邮箱**: abelxiaoxing@qq.com

---

<div align="center">

**⭐ 如果这个项目对您有帮助，请给我们一个 Star！ ⭐**

[![GitHub stars](https://img.shields.io/github/stars/your-repo/BiliCacheMerge.svg?style=social&label=Star)](https://github.com/your-repo/BiliCacheMerge)
[![GitHub forks](https://img.shields.io/github/forks/your-repo/BiliCacheMerge.svg?style=social&label=Fork)](https://github.com/your-repo/BiliCacheMerge/fork)

Made with ❤️ by [abelxiaoxing](https://github.com/abelxiaoxing)

</div>
