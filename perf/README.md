# WWJSON 性能测试框架

本目录包含 WWJSON 库的性能测试框架。

## 概述

性能测试框架旨在针对各种性能指标对 WWJSON 库与其他 JSON 库（主要是 yyjson）进行基准测试，包括：

- JSON 序列化速度
- 内存使用情况
- 不同数据大小的吞吐量

## 结构

- `test_data.h/.cpp`：生成各种大小测试数据的函数
- `pfwwjson`：主要的性能测试可执行文件

## 测试数据生成

框架包含生成不同大小 JSON 数据的函数：
- 0.5k、1k、10k、100k、500k、1M
- 使用 WWJSON 库的 `RawBuilder` 生成
- 通过大小容差检查进行验证

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

### 运行测试

```bash
# 列出所有可用测试
./build-release/perf/pfwwjson --List

# 运行特定测试
./build-release/perf/pfwwjson test_name

# 运行所有测试（包括工具类测试）
./build-release/perf/pfwwjson --all
```

### 使用 Make 包装器

```bash
# 构建并运行性能测试
make release

# 运行性能测试
make perf

# 仅重新构建性能测试
make build/perf
```

## 测试数据文件

生成的 JSON 测试数据写入 `perf/test_data.tmp/` 目录：
- 按大小命名的文件（例如 `0.5k.json`、`1k.json` 等）
- 用于手动检查和验证

生成测试数据示例：
```bash
./build-release/perf/pfwwjson data_sample
```

## 依赖项

- **couttast**：测试框架
- **yyjson**：性能比较库
- **WWJSON**：被测试的库

## 配置选项

- `WWJSON_LIB_ONLY`：仅构建库，不构建测试（默认：OFF）
- `BUILD_PERF_TESTS`：启用性能测试（默认：OFF）

## 未来扩展

此框架可以扩展到包括：
- 更全面的基准测试套件
- 内存性能分析
- 多线程性能测试
- 与其他 JSON 库的比较