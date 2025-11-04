#!/bin/bash

# ===================================================================
# Qt BiliCacheMerge 运行脚本
# 作者: 哈雷酱大小姐
# 描述: 快速启动Qt版本的B站缓存合并工具
# ===================================================================

# 颜色定义
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# 设置环境
setup_environment() {
    # 设置Qt环境变量
    export PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/bin:$PATH"
    export CMAKE_PREFIX_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64"
    export LD_LIBRARY_PATH="/home/abelxiaoxing/Qt/6.9.2/gcc_64/lib:$LD_LIBRARY_PATH"

    log_info "Qt环境已配置"
}

# 检查可执行文件
check_executable() {
    local executable="build/bin/BiliCacheMerge"

    if [[ ! -f "$executable" ]]; then
        log_warning "可执行文件不存在: $executable"
        log_info "请先运行 ./build.sh 进行编译"
        exit 1
    fi

    log_success "找到可执行文件: $executable"
}

# 运行程序
run_application() {
    local executable="build/bin/BiliCacheMerge"

    log_info "启动 Qt BiliCacheMerge..."
    echo "================================"

    # 启动应用
    "$executable" "$@"
}

# 显示帮助
show_help() {
    echo "用法: $0 [选项] [应用参数]"
    echo
    echo "选项:"
    echo "  help     显示此帮助信息"
    echo "  (无参数) 直接启动应用程序"
    echo
    echo "示例:"
    echo "  $0                    # 启动应用"
    echo "  $0 /path/to/videos    # 启动并指定视频目录"
    echo
}

# 主函数
main() {
    # 检查参数
    if [[ "$1" == "help" ]] || [[ "$1" == "-h" ]] || [[ "$1" == "--help" ]]; then
        show_help
        exit 0
    fi

    # 检查是否在正确的目录
    if [[ ! -f "CMakeLists.txt" ]]; then
        echo "错误: 请在qt_src目录下运行此脚本"
        exit 1
    fi

    # 执行启动流程
    setup_environment
    check_executable
    run_application "$@"
}

# 脚本入口
main "$@"