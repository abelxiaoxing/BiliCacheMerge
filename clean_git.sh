#!/bin/zsh

# ===================================================================
# Git 仓库优化脚本 - 删除历史中的大文件
# 作者: 哈雷酱大小姐
# 描述: 从git历史中移除大文件，减小.git文件夹体积
# ===================================================================

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
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

log_error() {
  echo -e "${RED}[ERROR]${NC} $1"
}

log_step() {
  echo -e "${PURPLE}[STEP]${NC} $1"
}

show_header() {
  echo -e "${PURPLE}"
  echo "╔══════════════════════════════════════════════════════════════╗"
  echo "║              Git 仓库大文件清理工具                           ║"
  echo "║                     by 哈雷酱大小姐                          ║"
  echo "╚══════════════════════════════════════════════════════════════╝"
  echo -e "${NC}"
}

show_git_size() {
  log_step "当前Git仓库大小分析..."

  local git_size=$(du -sh .git/ 2>/dev/null | cut -f1)
  echo "  .git/ 文件夹大小: $git_size"

  git count-objects -vH 2>/dev/null | grep -E "count:|size:" || true
  echo
}

find_large_files() {
  log_step "查找历史中的大文件..."

  # 查找大于10MB的文件
  echo "  正在扫描历史提交..."

  git rev-list --objects --all |
    git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' |
    awk '/^blob/ {print substr($0,6)}' |
    sort -k2 -n |
    tail -20 >/tmp/large_files.txt

  echo "  最大的20个文件："
  cat /tmp/large_files.txt | awk '{printf "    %8s  %s\n", $2, $3}' | head -10
  echo
}

option_1_filter_branch() {
  log_step "方案1: 使用 git filter-branch 删除指定文件..."

  log_info "此方法会重写git历史，移除匹配模式的文件"
  log_warning "⚠️  注意：这会改变git历史，如果已推送到远端，需要强制推送！"

  echo
  read -p "输入要删除的文件模式 (如: ffmpeg/* 或 *.exe): " pattern
  pattern=${pattern:-"ffmpeg/*"}

  echo
  log_info "执行命令: git filter-branch --force --index-filter \\"
  log_info "  'git rm --cached --ignore-unmatch $pattern' \\"
  log_info "  --prune-empty --tag-name-filter cat -- --all"

  read -p "确认执行？[y/N]: " confirm
  if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]; then
    git filter-branch --force --index-filter \
      "git rm --cached --ignore-unmatch $pattern" \
      --prune-empty --tag-name-filter cat -- --all

    log_step "清理引用..."
    rm -rf .git/refs/original/
    git reflog expire --expire=now --all
    git gc --prune=now --aggressive

    log_success "filter-branch完成！"
  else
    log_info "已取消"
    return 1
  fi
}

option_2_gc_optimize() {
  log_step "方案2: 基础优化 - 打包和清理..."

  log_info "此方法仅优化现有对象，不删除历史"

  echo
  log_info "正在执行 git gc..."

  # 打包现有对象
  git gc --prune=now

  # 强制优化
  git gc --aggressive --prune=now

  log_success "优化完成！"
}

option_3_backup_reinit() {
  log_step "方案3: 备份代码并重新初始化Git..."

  log_warning "⚠️  注意：这会删除所有git历史！"

  echo
  log_info "将保留当前文件，但删除所有提交历史"

  read -p "确认执行？[y/N]: " confirm
  if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]; then

    # 备份当前文件（排除.git）
    log_info "备份当前文件..."
    tar -czf /tmp/project_backup_$(date +%Y%m%d_%H%M%S).tar.gz \
      --exclude='.git' \
      --exclude='build' \
      .

    log_info "删除.git目录..."
    rm -rf .git

    log_info "重新初始化Git..."
    git init
    git add .
    git commit -m "Initial commit"

    log_success "Git重新初始化完成！"
    echo "  备份文件保存在: /tmp/project_backup_*.tar.gz"
  else
    log_info "已取消"
    return 1
  fi
}

option_4_bfg() {
  log_step "方案4: 使用 BFG Repo-Cleaner (最推荐)..."

  log_info "BFG是专门清理Git仓库大文件的工具，更快更安全"

  echo

  if ! command -v java &>/dev/null; then
    log_warning "需要安装Java才能使用BFG"
    echo "  Ubuntu/Debian: sudo apt-get install openjdk-11-jre-headless"
    echo "  macOS: brew install openjdk@11"
    return 1
  fi

  # 检查是否下载了BFG
  if [ ! -f "bfg.jar" ]; then
    log_info "下载 BFG Repo-Cleaner..."
    wget -c https://repo1.maven.org/maven2/com/madgag/bfg/1.14.0/bfg-1.14.0.jar -O bfg.jar ||
      curl -L -o bfg.jar https://repo1.maven.org/maven2/com/madgag/bfg/1.14.0/bfg-1.14.0.jar
    log_success "BFG下载完成"
  fi

  echo
  log_info "BFG使用方法："
  echo "  1. 删除大于50MB的文件:"
  echo "     java -jar bfg.jar --strip-blobs-bigger-than 50M ."
  echo
  echo "  2. 删除特定文件:"
  echo "     java -jar bfg.jar --delete-files '*.exe' ."
  echo "     java -jar bfg.jar --delete-files 'ffmpeg/*' ."
  echo
  echo "  3. 删除完成后清理:"
  echo "     git reflog expire --expire=now --all"
  echo "     git gc --prune=now --aggressive"

  read -p "输入要删除的文件模式 (回车跳过): " pattern

  if [ -n "$pattern" ]; then
    log_step "执行 BFG 清理..."

    # 删除指定的文件
    java -jar bfg.jar --delete-files "$pattern" .

    log_step "清理引用..."
    git reflog expire --expire=now --all
    git gc --prune=now --aggressive

    log_success "BFG清理完成！"
  else
    log_info "跳过BFG操作"
  fi
}

show_final_size() {
  log_step "清理后大小..."

  local git_size=$(du -sh .git/ 2>/dev/null | cut -f1)
  echo "  .git/ 文件夹大小: $git_size"

  git count-objects -vH 2>/dev/null | grep -E "size:|count:" || true
  echo

  log_success "Git仓库优化完成！"
}

show_menu() {
  echo
  echo "选择优化方案："
  echo
  echo "  1) 使用 git filter-branch 删除指定文件 (重写历史)"
  echo "  2) 基础优化 - git gc 打包清理"
  echo "  3) 备份并重新初始化Git (删除所有历史)"
  echo "  4) 使用 BFG Repo-Cleaner (最推荐)"
  echo "  0) 退出"
  echo
}

main() {
  show_header

  show_git_size

  find_large_files

  while true; do
    show_menu
    read -p "请选择 [0-4]: " choice

    case $choice in
    1)
      option_1_filter_branch && break
      ;;
    2)
      option_2_gc_optimize && break
      ;;
    3)
      option_3_backup_reinit && break
      ;;
    4)
      option_4_bfg && break
      ;;
    0)
      log_info "已退出"
      exit 0
      ;;
    *)
      log_error "无效选择"
      ;;
    esac
  done

  show_final_size

  echo
  log_info "额外提示："
  echo "  - 如果已推送到远端，使用 git push --force 强制推送"
  echo "  - 团队成员需要重新克隆仓库"
  echo "  - 备份重要代码！"
}

main "$@"
