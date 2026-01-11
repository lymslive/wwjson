# WWJSON 性能测试框架

本目录包含 WWJSON 库的性能测试框架。

## 概述

性能测试框架旨在针对各种性能指标对 WWJSON 库与其他 JSON 库（主要是 yyjson）进行基准测试，包括：

- JSON 序列化速度
- 不同数据大小的吞吐量

## 结构

- `p_builder.cpp` - JSON 构建器性能测试
- `p_number.cpp` - 数字序列化性能测试
- `p_string.cpp` - 字符串序列化性能测试
- `p_api.cpp` - 不同 api 风格的性能测试
- `p_design.cpp` - 设计选择性能测试
- `p_nodom.cpp` - 无 DOM 方式的 JSON 拼装性能测试
- `argv.h` - 命令行参数处理
- `relative_perf.h` - 相对性能测试框架
- `pfwwjson` - 主要的性能测试可执行文件

## 用例管理

详细的用例列表请参考 [cases.md](./cases.md)，其中列出了所有性能测试用例的名称和描述。

用例分类：
- `DEF_TAST` - 相对性能测试，默认自动运行，用于比较 wwjson 与 yyjson 的性能
- `DEF_TOOL` - 工具类测试，需要显式指定用例名才会运行，适合绝对性能测试和辅助工具

用例命名规范：
- 同一文件内的用例使用相同的前缀，一般与文件名相同
- 描述使用简洁的中文表达

## 使用方法

### 构建性能测试

```bash
# 配置启用性能测试
mkdir -p build-release
cd build-release
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_PERF_TESTS=ON ..

# 构建性能测试
make
```

### make 封装快捷命令

```bash
# 构建并运行性能测试
make release

# 运行性能测试
make perf

# 仅重新构建性能测试
make build/perf
```

### 运行测试

```bash
# 列出所有可用测试
./build-release/perf/pfwwjson --List

# 运行所有相对性能测试（DEF_TAST，默认行为）
./build-release/perf/pfwwjson

# 运行特定测试（支持多个用例名）
./build-release/perf/pfwwjson test_name1 test_name2

# 运行所有测试（包括工具类测试）
./build-release/perf/pfwwjson --all
```

### 命令行参数说明

性能测试用例支持以下命令行参数：

- `--loop=n` - 循环次数，默认 1000
- `--items=n` - 生成的项目数量，默认 1000
- `--start=n` - 起始值，用作随机种子或序列起始，默认 1
- `--size=n` - 预估的 JSON 大小（KB），默认 1（自动估算）

## GitHub Actions 自动化测试

GitHub Actions 工作流 `.github/workflows/ci-perf.yml` 提供了自动化的性能测试，可以在远程 CI 环境中运行性能测试。

### 使用 gh 命令行工具

如果已安装 `gh` 命令行工具，可以通过以下命令触发和管理 CI 工作流：

#### 触发工作流

```bash
# 在 dev 分支触发性能测试工作流（使用默认参数）
gh workflow run ci-perf.yml --ref dev

# 在 main 分支触发工作流
gh workflow run ci-perf.yml --ref main

# 自定义 CMake 配置参数
gh workflow run ci-perf.yml --ref dev \
  -f cmake_args="-DCMAKE_BUILD_TYPE=Release -DBUILD_PERF_TESTS=ON"

# 自定义测试参数
gh workflow run ci-perf.yml --ref dev \
  -f test_args="--cout=silent --verbose"
```

#### 查看工作流状态

```bash
# 列出 ci-perf.yml 的所有运行记录
gh run list --workflow=ci-perf.yml

# 查看特定运行的详细状态
gh run view <run-id>

# 实时监控运行进度
gh run watch <run-id>
```

#### 下载工作流日志

```bash
# 下载完整日志
gh run view <run-id> --log > perf/report.log/ci-<run-id>-full.log

# 只查看失败的步骤
gh run view <run-id> --log-failed
```

### 自动化脚本

项目提供了以下自动化脚本：

#### run-ci.sh

`perf/run-ci.sh` 脚本可以自动化整个 CI 流程：

```bash
# 运行脚本：触发 CI、等待完成、下载和提取日志
./perf/run-ci.sh
```

该脚本会：
1. 在 dev 分支触发 ci-perf.yml 工作流
2. 等待工作流运行完成
3. 下载完整日志到 `perf/report.log/ci-{run-id}-full.log`
4. 提取性能测试步骤的关键输出，生成精简日志 `perf/report.log/ci-yyyymmdd-hhmmss.log`

#### extract-perf-log.pl

`perf/extract-perf-log.pl` 脚本可以从 GitHub Actions 的全量日志中提取性能测试日志：

```bash
# 从全量日志提取性能测试日志
perl perf/extract-perf-log.pl ci-20881321775-full.log ci-20260111-004613.log

# 或输出到标准输出
perl perf/extract-perf-log.pl ci-20881321775-full.log > perf.log
```

该脚本会：
1. 定位 "Run performance tests" 步骤
2. 去除日志前缀（时间戳、步骤名等）
3. 只保留 pfwwjson 程序的实际输出
4. 自动提取时间戳并转换为本地时间格式（UTC+8）

日志文件保存在 `perf/report.log/` 目录下。

