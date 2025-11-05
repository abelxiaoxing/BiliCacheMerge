#!/bin/bash

# ===================================================================
# FFmpeg 精简编译脚本
# 作者: 哈雷酱大小姐
# 描述: 编译体积超小的FFmpeg仅用于B站缓存合并
# ===================================================================

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m'

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

show_header() {
    echo -e "${PURPLE}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║                FFmpeg 精简编译脚本                           ║"
    echo "║                     by 哈雷酱大小姐                          ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${NC}"
}

check_system_ffmpeg() {
    log_step "检查系统FFmpeg..."

    if command -v ffmpeg &> /dev/null; then
        local ffmpeg_size=$(stat -c%s $(which ffmpeg))
        local ffprobe_size=$(stat -c%s $(which ffprobe))

        if [ $ffmpeg_size -lt 2000000 ] && [ $ffprobe_size -lt 2000000 ]; then
            log_warning "检测到系统FFmpeg已足够精简"
            log_info "ffmpeg大小: $((ffmpeg_size / 1024)) KB"
            log_info "ffprobe大小: $((ffprobe_size / 1024)) KB"
            log_info "当前方案就是最优解，无需重新编译！"
            log_info ""
            log_info "使用命令复制系统版本："
            echo "  cp \$(which ffmpeg) ./ffmpeg/ffmpeg"
            echo "  cp \$(which ffprobe) ./ffmpeg/ffprobe"
            return 1
        fi
    fi
    return 0
}

download_ffmpeg() {
    log_step "下载FFmpeg源码..."

    local version="${1:-4.4.4}"
    local filename="ffmpeg-${version}.tar.xz"
    local url="https://ffmpeg.org/releases/${filename}"

    if [ -f "$filename" ]; then
        log_success "源码文件已存在"
        return 0
    fi

    log_info "正在下载 $version 版本..."
    wget -c "$url" || {
        log_error "下载失败"
        return 1
    }

    log_success "下载完成"
}

compile_ffmpeg() {
    log_step "配置编译选项..."

    local version="${1:-4.4.4}"
    local srcdir="ffmpeg-${version}"

    # 解压源码
    tar -xf "${filename}"

    cd "$srcdir"

    # 超精简配置 - 只保留B站需要的核心功能
    local config_flags="
        --prefix=$(pwd)/../ffmpeg_mini_build
        --enable-static
        --disable-shared
        --disable-programs
        --disable-doc
        --disable-htmlpages
        --disable-manpages
        --disable-podpages
        --disable-txtpages
        --disable-avdevice
        --disable-devices
        --disable-filters
        --disable-parsers
        --disable-demuxers
        --disable-muxers
        --disable-protocols
        --disable-bsfs
        --disable-indevs
        --disable-outdevs
        --enable-hwaccels
        --enable-hwaccel=h264
        --enable-hwaccel=h265
        --enable-decoder=h264
        --enable-decoder=hevc
        --enable-decoder=h265
        --enable-decoder=aac
        --enable-decoder=mp3
        --enable-decoder=opus
        --enable-decoder=mp3
        --enable-demuxer=mov
        --enable-demuxer=matroska
        --enable-muxer=mp4
        --enable-muxer=mp3
        --enable-muxer=mov
        --disable-encoders
        --disable-muxers=all
        --disable-demuxers=all
        --disable-decoders=all
        --enable-demuxer=mov
        --enable-demuxer=matroska
        --enable-muxer=mp4
        --enable-decoder=h264
        --enable-decoder=aac
        --enable-decoder=mp3
        --disable-parser=h264
        --disable-parser=aac
        --disable-protocols
        --disable-zlib
        --disable-bzlib
        --disable-lzma
        --disable-zstd
        --disable-libxcb
        --disable-sdl2
        --disable-iconv
        --disable-sndio
        --disable-sndio
        --disable-xlib
        --disable-cuda
        --disable-cuvid
        --disable-nvenc
        --disable-vaapi
        --disable-vdpau
        --disable-v4l2-m2m
        --disable-openal
        --disable-opengl
        --disable-vulkan
        --disable-libglvnd
    "

    log_info "执行配置..."

    # 尝试不同的配置级别
    if ! ./configure $config_flags 2>&1 | tee configure.log; then
        log_warning "精简配置失败，尝试基础配置..."

        # 基础配置
        local basic_flags="
            --prefix=$(pwd)/../ffmpeg_mini_build
            --enable-static
            --disable-shared
            --disable-programs
            --disable-doc
            --disable-htmlpages
            --disable-manpages
            --disable-podpages
            --disable-txtpages
            --disable-avdevice
            --disable-devices
            --disable-sdl2
            --disable-zlib
            --disable-bzlib
            --disable-iconv
            --enable-hwaccel=h264
            --enable-hwaccel=h265
        "

        ./configure $basic_flags 2>&1 | tee configure.log
    fi

    log_success "配置完成"

    # 显示编译信息
    log_step "显示关键配置信息..."
    grep -E "(Enabled encoders|Enabled decoders|Enabled muxers|Enabled demuxers)" config.log | head -10

    log_step "开始编译..."
    make -j$(nproc)

    log_step "安装..."
    make install

    log_success "编译完成！"
}

verify_result() {
    log_step "验证编译结果..."

    local ffmpeg_path="ffmpeg_mini_build/bin/ffmpeg"
    local ffprobe_path="ffmpeg_mini_build/bin/ffprobe"

    if [ -f "$ffmpeg_path" ] && [ -f "$ffprobe_path" ]; then
        local ffmpeg_size=$(stat -c%s "$ffmpeg_path")
        local ffprobe_size=$(stat -c%s "$ffprobe_path")

        log_success "编译成功！"
        echo
        echo "文件大小对比："
        echo "  优化前: ffmpeg=135MB, ffprobe=135MB"
        echo "  优化后: ffmpeg=$((ffmpeg_size / 1024))KB, ffprobe=$((ffprobe_size / 1024))KB"
        echo "  节省空间: $(( (270000 - (ffmpeg_size + ffprobe_size) / 1000) ))KB"
        echo
        echo "安装到项目目录："
        echo "  cp $ffmpeg_path ../ffmpeg/ffmpeg"
        echo "  cp $ffprobe_path ../ffmpeg/ffprobe"
        echo

        # 测试关键编解码器
        log_step "测试关键编解码器..."
        "$ffmpeg_path" -codecs 2>/dev/null | grep -E "h264|aac" | head -5

    else
        log_error "编译失败：可执行文件未生成"
        return 1
    fi
}

main() {
    show_header

    echo "本脚本提供两种优化方案："
    echo
    echo "方案1：使用系统FFmpeg（推荐★★★★★）"
    echo "  - 优点：无需编译，直接使用动态链接版本，大小仅~500KB"
    echo "  - 适用：大多数Linux系统"
    echo "  - 命令："
    echo "    cp \$(which ffmpeg) ./ffmpeg/ffmpeg"
    echo "    cp \$(which ffprobe) ./ffmpeg/ffprobe"
    echo
    echo "方案2：从源码编译静态精简版"
    echo "  - 优点：完全静态，无依赖"
    echo "  - 缺点：需要编译时间，文件仍较大（~30-50MB）"
    echo
    read -p "选择方案 [1/2] (默认1): " choice
    choice=${choice:-1}

    if [ "$choice" = "2" ]; then
        check_system_ffmpeg && {
            echo
            read -p "系统FFmpeg已足够精简，是否仍要重新编译？[y/N]: " confirm
            [ "$confirm" != "y" ] && [ "$confirm" != "Y" ] && exit 0
        }

        echo
        read -p "输入FFmpeg版本 (默认4.4.4): " version
        version=${version:-4.4.4}

        download_ffmpeg "$version"
        compile_ffmpeg "$version"
        verify_result
    else
        log_step "使用系统FFmpeg..."
        if command -v ffmpeg &> /dev/null && command -v ffprobe &> /dev/null; then
            cp $(which ffmpeg) ./ffmpeg/ffmpeg
            cp $(which ffprobe) ./ffmpeg/ffprobe

            log_success "已复制系统FFmpeg到项目目录"
            ls -lh ./ffmpeg/ffmpeg ./ffmpeg/ffprobe
        else
            log_error "系统未安装FFmpeg"
            exit 1
        fi
    fi

    log_success "优化完成！"
}

main "$@"
