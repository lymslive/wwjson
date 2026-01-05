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

