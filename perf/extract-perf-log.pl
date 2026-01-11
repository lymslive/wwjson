#!/usr/bin/perl
#
# extract-perf-log.pl - 从 GitHub Actions 全量日志中提取性能测试日志
#
# 用法：
#   perl extract-perf-log.pl <full-log-file> [output-file]
#
# 功能：
#   1. 从全量日志中定位 "Run performance tests" 步骤
#   2. 去掉日志前缀（前三列），只保留程序输出
#   3. 提取时间戳并转换为本地时间格式（UTC+8）
#   4. 生成精简的性能测试日志
#
# 如果未指定 output-file，则输出到标准输出

use strict;
use warnings;
use POSIX qw(strftime);
use Time::Local qw(timelocal);

# 颜色输出（可选）
my $RED = "\e[31m";
my $GREEN = "\e[32m";
my $YELLOW = "\e[33m";
my $NC = "\e[0m";  # No Color

# 检查参数
if (@ARGV < 1 || @ARGV > 2) {
    print STDERR "${YELLOW}用法${NC}: perl extract-perf-log.pl <full-log-file> [output-file]\n";
    print STDERR "\n示例:\n";
    print STDERR "  perl extract-perf-log.pl ci-20881321775-full.log ci-20260111-004613.log\n";
    print STDERR "  perl extract-perf-log.pl ci-20881321775-full.log > perf.log\n";
    exit 1;
}

my $input_file = $ARGV[0];
my $output_file = defined $ARGV[1] ? $ARGV[1] : '';

# 打开输入文件
open(my $fh, '<', $input_file) or die "${RED}错误${NC}: 无法打开输入文件 '$input_file': $!\n";

# 读取全部行
my @lines = <$fh>;
close($fh);

# 定位 "Run performance tests" 步骤的起始和结束行
my $step_start = -1;
my $step_end = -1;

for (my $i = 0; $i < @lines; $i++) {
    my $line = $lines[$i];

    # 查找步骤开始
    if ($line =~ /^performance-tests\tRun performance tests\t/) {
        $step_start = $i;
        last;
    }
}

# 如果没找到步骤开始
if ($step_start == -1) {
    print STDERR "${RED}错误${NC}: 未找到 'Run performance tests' 步骤\n";
    exit 1;
}

# 查找步骤结束（下一个以 "performance-tests\t" 开头的行，但不是 Run performance tests）
for (my $i = $step_start + 1; $i < @lines; $i++) {
    my $line = $lines[$i];

    # 查找下一个步骤的开始（performance-tests + tab + 步骤名）
    # 注意：下一个步骤可能是 "Post Checkout code"，所以需要匹配任何包含 "performance-tests\t" 的行
    if ($line =~ /^performance-tests\t/ && $line !~ /^performance-tests\tRun performance tests\t/) {
        $step_end = $i - 1;  # 结束行是下一个步骤的前一行
        last;
    }
}

# 如果没找到步骤结束，读取到文件末尾
if ($step_end == -1) {
    $step_end = @lines - 1;
}

# print STDERR "${GREEN}找到 'Run performance tests' 步骤${NC}: 行 $step_start+1 到 $step_end+1\n";
print STDERR "${GREEN}找到 'Run performance tests' 步骤${NC}: 行 " . ($step_start + 1) . " 到 " . ($step_end + 1) . "\n";

# 提取时间戳（从步骤的第一行）
my $timestamp_line = $lines[$step_start];
my ($job_name, $step_name, $timestamp_str) = split(/\t/, $timestamp_line);

# 时间戳格式: 2026-01-10T16:46:13.3186685Z
if ($timestamp_str =~ /^(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})/) {
    my ($year, $month, $day, $hour, $min, $sec) = ($1, $2, $3, $4, $5, $6);

    # 将 UTC 时间转换为本地时间（假设是 UTC+8）
    # 直接加 8 小时
    my $time = timelocal($sec, $min, $hour, $day, $month - 1, $year);
    $time += 8 * 3600;  # 加 8 小时

    # 格式化为 yyyymmdd-hhmmss
    my $local_time = strftime("%Y%m%d-%H%M%S", localtime($time));

    print STDERR "${GREEN}时间戳${NC}: $timestamp_str (UTC) -> $local_time (UTC+8)\n";
}

# 提取并处理步骤内容
my @perf_lines;
my $found_program_output = 0;

for (my $i = $step_start; $i <= $step_end; $i++) {
    my $line = $lines[$i];
    chomp($line);

    # 查找 "Running command:" 行，这是程序输出的开始
    if ($line =~ /Running command: .*pfwwjson/) {
        $found_program_output = 1;
        # 从这一行开始提取
        my @parts = split(/\t/, $line);
        if (@parts >= 3) {
            # 去掉时间戳，保留实际内容
            # 时间戳格式: 2026-01-10T16:46:13.3186685Z
            my $content = $parts[2];
            $content =~ s/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d+Z\s+//;
            push @perf_lines, $content if $content;
        }
        next;
    }

    # 如果还没找到程序输出，跳过
    next unless $found_program_output;

    # 提取内容（去掉时间戳）
    my @parts = split(/\t/, $line);
    if (@parts >= 3) {
        my $content = $parts[2];

        # 去掉时间戳前缀
        $content =~ s/^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d+Z\s+//;

        # 跳过包含 ANSI 转义码的控制行（如 shell: /usr/bin/bash -e {0}）
        next if $content =~ /^\e\[.*?m.*$/;
        next if $content =~ /^shell:/;
        next if $content =~ /^env:/;
        next if $content =~ /^##\[group\]/;
        next if $content =~ /^##\[endgroup\]/;
        next if $content =~ /^\[36;1m.*\[0m$/;  # 带颜色的命令行
        next if $content =~ /^Post job cleanup/;  # 跳过 Post job cleanup

        # 保留空行（pfwwjson 框架设计的空行，增加可读性）
        # 只保留非控制字符的空行
        if ($content eq '') {
            push @perf_lines, $content;
            next;
        }

        push @perf_lines, $content;
    }
}

print STDERR "${GREEN}提取${NC} " . scalar(@perf_lines) . " 行性能测试日志\n";

# 输出结果
my $output_fh;
if ($output_file) {
    open($output_fh, '>', $output_file) or die "${RED}错误${NC}: 无法创建输出文件 '$output_file': $!\n";
    print STDERR "${GREEN}输出到${NC}: $output_file\n";
} else {
    $output_fh = \*STDOUT;
}

# 写入日志
foreach my $line (@perf_lines) {
    print $output_fh $line . "\n";
}

if ($output_file) {
    close($output_fh);
    print STDERR "${GREEN}完成！${NC}\n";
}
