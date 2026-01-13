#!/bin/bash

# run-ci.sh - 自动运行 GitHub Actions CI 性能测试并提取日志
#
# 功能：
# 1. 触发 ci-perf.yml 工作流（在 dev 分支）
# 2. 等待工作流运行完成
# 3. 下载完整日志到 perf/report.log/ci-{run-id}-full.txt
# 4. 提取性能测试步骤的关键输出，生成精简日志 ci-yyyymmdd-hhmmss.log

set -e  # 遇到错误立即退出

# 全局变量（用于在函数间传递值）
RUN_ID=""
FULL_LOG_FILE=""
TIMESTAMP=""
PERF_LOG_FILE=""
SHOULD_TRIGGER=true  # 默认触发工作流

# 配置参数
WORKFLOW="ci-perf.yml"
BRANCH="dev"
LOG_DIR="perf/report.log"
POLL_INTERVAL=10  # 轮询间隔（秒）
TIMEOUT=3600      # 超时时间（秒），1小时

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 日志函数
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1" >&2
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1" >&2
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

# 解析命令行参数
parse_args() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            --no-run)
                SHOULD_TRIGGER=false
                shift
                ;;
            -h|--help)
                echo "用法: $0 [选项]"
                echo "选项:"
                echo "  --no-run       不触发新工作流，只处理最近运行的流水线"
                echo "  -h, --help     显示此帮助信息"
                exit 0
                ;;
            *)
                log_error "未知选项: $1"
                echo "使用 $0 --help 查看可用选项"
                exit 1
                ;;
        esac
    done
}

# 检查依赖
check_dependencies() {
    if ! command -v gh &> /dev/null; then
        log_error "gh 命令未安装，请先安装 GitHub CLI"
        exit 1
    fi

    if ! gh auth status &> /dev/null; then
        log_error "GitHub CLI 未登录，请运行: gh auth login"
        exit 1
    fi

    if ! command -v jq &> /dev/null; then
        log_error "jq 命令未安装，请先安装 jq 用于 JSON 解析"
        exit 1
    fi
}

# 确保日志目录存在
ensure_log_dir() {
    if [ ! -d "$LOG_DIR" ]; then
        log_info "创建日志目录: $LOG_DIR"
        mkdir -p "$LOG_DIR"
    fi
}

# 触发工作流
trigger_workflow() {
    log_info "触发工作流: $WORKFLOW (分支: $BRANCH)"

    local result
    result=$(gh workflow run "$WORKFLOW" --ref "$BRANCH" 2>&1)

    if [ $? -ne 0 ]; then
        log_error "触发工作流失败: $result"
        exit 1
    fi

    # 等待一小段时间让工作流启动
    sleep 2
}

# 获取最新的 run-id
get_latest_run_id() {
    log_info "获取工作流运行 ID..."

    local run_id
    run_id=$(gh run list --workflow="$WORKFLOW" --branch="$BRANCH" --limit 1 --json databaseId --jq '.[0].databaseId' 2>/dev/null)

    if [ -z "$run_id" ] || [ "$run_id" = "null" ]; then
        log_error "无法获取 run-id"
        exit 1
    fi

    # 设置全局变量
    RUN_ID="$run_id"
}

# 等待工作流完成
wait_for_completion() {
    local run_id=$1
    local elapsed=0

    log_info "等待工作流运行完成 (run-id: $run_id)..."

    while [ $elapsed -lt $TIMEOUT ]; do
        local status
        status=$(gh run view "$run_id" --json status --jq '.status' 2>&1)

        if [ "$status" = "completed" ]; then
            local conclusion
            conclusion=$(gh run view "$run_id" --json conclusion --jq '.conclusion' 2>&1)

            if [ "$conclusion" = "success" ]; then
                log_info "工作流运行成功!"
                return 0
            else
                log_warn "工作流运行失败 (conclusion: $conclusion)"
                return 1
            fi
        elif [ "$status" = "failure" ]; then
            log_error "工作流运行失败"
            return 1
        fi

        log_info "工作流状态: $status, 已等待 ${elapsed}s, 继续等待..."
        sleep $POLL_INTERVAL
        elapsed=$((elapsed + POLL_INTERVAL))
    done

    log_error "等待超时 (${TIMEOUT}s)"
    return 1
}

# 下载完整日志
download_full_log() {
    local run_id=$1
    local full_log_file="$LOG_DIR/ci-${run_id}-full.txt"

    log_info "下载完整日志到: $full_log_file"

    gh run view "$run_id" --log > "$full_log_file" 2>/dev/null

    if [ $? -ne 0 ]; then
        log_error "下载日志失败"
        return 1
    fi

    log_info "日志下载成功 ($(wc -l < "$full_log_file") 行)"

    # 设置全局变量
    FULL_LOG_FILE="$full_log_file"
}

# 从日志中提取时间戳
extract_timestamp() {
    local full_log_file=$1

    # 在日志中查找 "Run performance tests" 步骤的第一行
    # 时间戳格式通常是: 2026-01-11T00:58:23.1234567Z
    # 转换为: 20260111-005823 (去掉小数秒，加 8 小时时区)

    local timestamp_line
    timestamp_line=$(grep -A 1 "Run performance tests" "$full_log_file" | tail -1 | grep -oE '[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}' | head -1)

    if [ -z "$timestamp_line" ]; then
        log_warn "无法从日志提取时间戳，使用当前时间"
        TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
        return
    fi

    # 使用 date 命令处理时区
    # GitHub Actions 日志使用 UTC 时间，需要转换为本地时间
    # 本地系统时区已经是 UTC+8 (CST)，date 命令会自动转换
    local formatted_time
    formatted_time=$(date -d "$timestamp_line UTC" +"%Y%m%d-%H%M%S" 2>/dev/null)

    if [ -z "$formatted_time" ]; then
        log_warn "时间戳转换失败，使用当前时间"
        TIMESTAMP=$(date +"%Y%m%d-%H%M%S")
    else
        TIMESTAMP="$formatted_time"
    fi
}

# 提取性能测试日志
extract_perf_log() {
    local full_log_file=$1
    local timestamp=$2
    PERF_LOG_FILE="$LOG_DIR/ci-${timestamp}.log"

    log_info "提取性能测试日志到: $PERF_LOG_FILE"

    # 使用 perl 脚本提取性能测试日志
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

    if [ ! -f "$script_dir/extract-perf-log.pl" ]; then
        log_error "未找到 extract-perf-log.pl 脚本"
        return 1
    fi

    # 执行 perl 脚本，捕获其输出，避免与 bash 脚本输出交错
    local perl_output
    echo "perf/extract-perf-log.pl $full_log_file $PERF_LOG_FILE"
    perl_output=$(perl "$script_dir/extract-perf-log.pl" "$full_log_file" "$PERF_LOG_FILE" 2>&1)
    local perl_exit=$?

    # 只有在 perl 脚本失败时才输出错误信息
    if [ $perl_exit -ne 0 ]; then
        log_error "日志提取失败"
        if [ -n "$perl_output" ]; then
            echo "Perl 脚本输出:" >&2
            echo "$perl_output" >&2
        fi
        return 1
    fi

    # perl 脚本成功
    echo "$perl_output" >&2
}

# 主函数
main() {
    # 解析命令行参数
    parse_args "$@"

    log_info "开始自动运行 GitHub Actions CI 性能测试..."

    # 检查依赖
    check_dependencies

    # 确保日志目录存在
    ensure_log_dir

    if [ "$SHOULD_TRIGGER" = true ]; then
        # 触发工作流
        trigger_workflow
        
        # 获取最新触发的 run-id
        get_latest_run_id
        log_info "工作流运行 ID: $RUN_ID"

        # 等待工作流完成
        if ! wait_for_completion "$RUN_ID"; then
            log_error "工作流运行失败，请查看日志"
            gh run view "$RUN_ID" --log-failed
            exit 1
        fi
    else
        # 不触发新工作流，获取最近已完成的 run-id
        log_info "不触发新工作流，处理最近运行的流水线..."
        get_latest_run_id
        log_info "工作流运行 ID: $RUN_ID"
        
        # 检查工作流状态
        local status
        status=$(gh run view "$RUN_ID" --json status --jq '.status' 2>&1)
        log_info "工作流状态: $status"
        
        if [ "$status" != "completed" ]; then
            log_warn "工作流状态为 $status，可能仍在运行中"
        fi
    fi

    # 下载完整日志
    download_full_log "$RUN_ID"

    # 提取时间戳
    extract_timestamp "$FULL_LOG_FILE"
    log_info "日志时间戳: $TIMESTAMP"

    # 提取性能测试日志
    extract_perf_log "$FULL_LOG_FILE" "$TIMESTAMP"

    # 输出结果摘要
    echo ""
    log_info "========== 处理完成 =========="
    log_info "工作流 ID: $RUN_ID"
    log_info "完整日志: $FULL_LOG_FILE"
    log_info "性能测试日志: $PERF_LOG_FILE"
    log_info "==============================="

    # 显示性能测试日志的前几行
    echo ""
    log_info "性能测试日志预览:"
    head -20 "$PERF_LOG_FILE"

    log_info "所有任务完成!"
}

# 运行主函数
main "$@"
