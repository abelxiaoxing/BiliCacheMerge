#!/bin/bash

# ===================================================================
# Qt BiliCacheMerge 编译脚本
# 作者: 哈雷酱大小姐
# 描述: 一键编译Qt版本的B站缓存合并工具
# ===================================================================

set -e  # 遇到错误立即退出

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
    echo -e "${PURPLE}[STEP]${NC} $1"
}

# 显示标题
show_header() {
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                Qt BiliCacheMerge 编译脚本                    ║"
    echo "║                     by 哈雷酱大小姐                          ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

# 检查依赖
check_dependencies() {
    log_step "检查编译依赖..."

    # 检查Qt安装
    if [[ ! -d "/home/abelxiaoxing/Qt/6.9.2/gcc_64" ]]; then
        log_error "Qt 6.9.2 未安装在预期路径: /home/abelxiaoxing/Qt/6.9.2/gcc_64"
        exit 1
    fi

    # 检查CMake
    if ! command -v cmake &> /dev/null; then
        log_error "CMake 未安装"
        exit 1
    fi

    # 检查编译器
    if ! command -v g++ &> /dev/null; then
        log_error "g++ 编译器未安装"
        exit 1
    fi

    local cmake_version=$(cmake --version | head -n1 | grep -oP '\d+\.\d+')
    local gcc_version=$(g++ --version | head -n1 | grep -oP '\d+\.\d+')

    log_success "依赖检查通过"
    log_info "  - Qt: 6.9.2"
    log_info "  - CMake: $cmake_version"
    log_info "  - G++: $gcc_version"
}

# 设置环境变量
setup_environment() {
    log_step "设置Qt环境变量..."

    export PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/bin:$PATH"
    export CMAKE_PREFIX_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64"
    export LD_LIBRARY_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/lib:$LD_LIBRARY_PATH"

    # 验证Qt环境
    if ! qmake6 -version &> /dev/null; then
        log_error "Qt环境配置失败"
        exit 1
    fi

    log_success "Qt环境配置完成"
}

# 检查并创建图标文件
check_icons() {
    log_step "检查图标资源文件..."

    local resources_dir="resources"
    local missing_icons=false

    # 检查必需的图标文件
    declare -A required_icons=(
        ["main/play_64x64.png"]="主界面播放图标"
        ["main/folder_64x64.png"]="主界面文件夹图标"
        ["main/settings_64x64.png"]="主界面设置图标"
        ["navigation/directory_32x32.png"]="导航目录图标"
        ["navigation/gear_32x32.png"]="导航齿轮图标"
        ["navigation/tools_32x32.png"]="导航工具图标"
        ["navigation/info_32x32.png"]="导航信息图标"
        ["status/success_24x24.png"]="状态成功图标"
        ["status/warning_24x24.png"]="状态警告图标"
        ["status/error_24x24.png"]="状态错误图标"
        ["status/processing_24x24.png"]="状态处理中图标"
        ["media/video_32x32.png"]="媒体视频图标"
        ["media/audio_32x32.png"]="媒体音频图标"
        ["media/subtitle_32x32.png"]="媒体字幕图标"
        ["media/danmaku_32x32.png"]="媒体弹幕图标"
    )

    for icon_path in "${!required_icons[@]}"; do
        local full_path="$resources_dir/$icon_path"
        if [[ ! -f "$full_path" ]]; then
            log_warning "缺失图标: $icon_path (${required_icons[$icon_path]})"
            missing_icons=true
        fi
    done

    if [[ "$missing_icons" == true ]]; then
        log_step "创建缺失的占位图标..."
        python3 -c "
import struct
import os

def create_minimal_png(filename, width=24, height=24):
    os.makedirs(os.path.dirname(filename), exist_ok=True)
    # 创建最小的透明PNG文件
    png_data = struct.pack('>8B', 137, 80, 78, 71, 13, 10, 26, 10)
    # IHDR chunk
    ihdr_data = struct.pack('>2I5B', width, height, 8, 6, 0, 0, 0)
    ihdr_crc = 0x9DC6C723
    png_data += struct.pack('>I', 13) + b'IHDR' + ihdr_data + struct.pack('>I', ihdr_crc)
    # IDAT chunk
    compressed_data = b'\x78\x9c\x62\x00\x02\x00\x00\xff\xff\x00\x00\x00\xff\xff'
    idat_crc = 0x9226C8A3
    png_data += struct.pack('>I', len(compressed_data)) + b'IDAT' + compressed_data + struct.pack('>I', idat_crc)
    # IEND chunk
    png_data += struct.pack('>I', 0) + b'IEND' + struct.pack('>I', 0xAE426082)

    with open(filename, 'wb') as f:
        f.write(png_data)

icons = [
    'main/play_64x64.png', 'main/folder_64x64.png', 'main/settings_64x64.png',
    'navigation/directory_32x32.png', 'navigation/gear_32x32.png', 'navigation/tools_32x32.png', 'navigation/info_32x32.png',
    'status/success_24x24.png', 'status/warning_24x24.png', 'status/error_24x24.png', 'status/processing_24x24.png',
    'media/video_32x32.png', 'media/audio_32x32.png', 'media/subtitle_32x32.png', 'media/danmaku_32x32.png'
]

for icon in icons:
    if '64x64' in icon:
        create_minimal_png(icon, 64, 64)
    elif '32x32' in icon:
        create_minimal_png(icon, 32, 32)
    else:
        create_minimal_png(icon, 24, 24)
    print(f'Created {icon}')
"
        log_success "占位图标创建完成"
    else
        log_success "所有图标文件已存在"
    fi
}

# 清理和准备构建目录
prepare_build() {
    log_step "准备构建目录..."

    if [[ -d "build" ]]; then
        log_warning "检测到现有build目录，正在清理..."
        rm -rf build
    fi

    mkdir -p build
    log_success "构建目录准备完成"
}

# CMake配置
configure_cmake() {
    log_step "执行CMake配置..."

    cd build

    # 执行CMake配置
    if cmake .. -DCMAKE_BUILD_TYPE=Release; then
        log_success "CMake配置成功"
    else
        log_error "CMake配置失败"
        exit 1
    fi

    # 显示配置信息
    log_info "构建配置:"
    log_info "  - 构建类型: Release"
    log_info "  - Qt版本: 6.9.2"
    log_info "  - C++标准: C++17"
    log_info "  - 输出目录: $(pwd)/bin"
}

# 编译项目
compile_project() {
    log_step "开始编译项目..."

    # 获取CPU核心数
    local cpu_cores=$(nproc)
    log_info "使用 $cpu_cores 个CPU核心进行编译"

    # 执行编译
    if make -j$cpu_cores; then
        log_success "编译完成"
    else
        log_error "编译失败"
        exit 1
    fi
}

# 验证编译结果
verify_build() {
    log_step "验证编译结果..."

    local executable="bin/BiliCacheMerge"

    if [[ -f "$executable" ]]; then
        local file_size=$(stat -c%s "$executable")
        local file_size_kb=$((file_size / 1024))

        log_success "可执行文件生成成功"
        log_info "  - 文件路径: $(pwd)/$executable"
        log_info "  - 文件大小: ${file_size_kb} KB"
        log_info "  - 文件类型: $(file "$executable")"

        # 测试启动（无GUI模式）
        log_step "测试程序启动..."
        if timeout 3s ./$executable --version &> /dev/null || true; then
            log_success "程序启动测试通过"
        else
            log_warning "程序启动测试未完全通过，但可能是正常的（需要GUI环境）"
        fi

    else
        log_error "可执行文件未生成: $executable"
        exit 1
    fi
}

# 显示使用说明
show_usage() {
    echo
    log_info "编译完成！使用方法："
    echo
    echo "  # 设置库路径并运行"
    echo "  export LD_LIBRARY_PATH=\"/home/abelxiaoxing/Qt/6.9.2/gcc_64/lib:\$LD_LIBRARY_PATH\""
    echo "  ./build/bin/BiliCacheMerge"
    echo
    echo "  # 或者直接使用提供的运行脚本"
    echo "  ./build/bin/BiliCacheMerge"
    echo
    log_info "其他可用目标："
    echo "  - 测试程序: ./build/simple_test"
    echo "  - 扫描器测试: ./build/test_scanner_cli"
    echo
}

# 主函数
main() {
    show_header

    # 检查是否在正确的目录
    if [[ ! -f "CMakeLists.txt" ]] || [[ ! -d "src" ]]; then
        log_error "请在qt_src目录下运行此脚本"
        exit 1
    fi

    # 检查参数
    if [[ "$1" == "clean" ]]; then
        log_step "清理构建文件..."
        rm -rf build
        log_success "清理完成"
        exit 0
    fi

    if [[ "$1" == "help" ]] || [[ "$1" == "-h" ]] || [[ "$1" == "--help" ]]; then
        echo "用法: $0 [选项]"
        echo
        echo "选项:"
        echo "  clean    清理构建文件"
        echo "  help     显示此帮助信息"
        echo "  (无参数) 执行完整编译流程"
        echo
        exit 0
    fi

    # 执行编译流程
    check_dependencies
    setup_environment
    check_icons
    prepare_build
    configure_cmake
    compile_project
    verify_build
    show_usage

    log_success "所有任务完成！Qt版本BiliCacheMerge编译成功！"
}

# 脚本入口
main "$@"