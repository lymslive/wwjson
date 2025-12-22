# 项目开发原始需求管理

本文档由人工维护，**AI受限编辑** ，仅允许在每个 `## TODO` 下加个子标题 `###
DONE` 关联 `task_log.md` 的完成任务ID记录。

需求ID 格式：`TODO:YYYY-MM-DD/n` ，日期/编号。日期表示头脑风暴冒出该需求想法的
日期，若一天中产出多个想法则加编号。仅有初步概念但未细化的需求暂不加日期 id ，
先快速列出 `TODO: 标题`，后面有细化内容时再赋日期 id ，转为正式需求。

---

## 上个版本 v1.0.0 需求摘要

WWJSON v1.0.0 已完成发布，主要成果包括：

### 核心功能
- ✅ 仅头文件 JSON 构建库，支持高性能直接字符串拼接
- ✅ 完整的 RAII 作用域管理（GenericArray/GenericObject）
- ✅ 模板化设计支持自定义字符串类型
- ✅ 配置化转义策略和数字序列化选项
- ✅ 多种 API 风格：传统方法、操作符重载、链式调用、Lambda 嵌套

### 性能优化
- ✅ 小整数缓存表优化，性能提升 10-15 倍
- ✅ 浮点数定点优化，四位小数序列化提升 20%
- ✅ 字符串转义优化，使用临时缓冲区避免频繁 push_back
- ✅ 完整的性能测试框架，与 yyjson 等主流库对比

### 开发工具
- ✅ CMake 构建系统和安装配置
- ✅ 75 个单元测试用例，覆盖所有核心功能
- ✅ 44 个性能测试用例，全面的基准测试
- ✅ CI/CD 流水线，自动化测试和文档部署

### 文档系统
- ✅ 完整的 Doxygen API 文档
- ✅ 详细的用户指南和示例
- ✅ 在线文档自动部署到 GitHub Pages

### 详细记录
- [changelog/v1.0/task_todo.md](changelog/v1.0/task_todo.md)
- [changelog/v1.0/task_log.md](changelog/v1.0/task_log.md)

---

**后续需求记录**

## TODO:2025-12-22/1 v1.0.0 封版发布

基础功能开发完毕，文档补完，可以打个标签了。
后续开发计划：
- 继续性能优化
- 开发专用的字符串类作为写入目标
- 扩展功能可能写在单独的头文件更合适，依然是仅头文件，但未必单头文件

### DONE: 20251222-110430

## TODO:2025-12-22/2 再测试探索整数序列化优化方法

疑问：用 memcpy 拷贝 2 个字符更好，还是写两次单字符赋值？

在 `perf/p_design.cpp` 文件中增加一个相对测试类 `test::perf::WriteUnsignedCompare`：
- 方法 B: 拷贝当前实现 `NumberWriter::WriteUnsigned` 作为测试基准
- 方法 A: 两次 `*(--ptr)` 赋值改为 `::memcpy(ptr -=2, src, 2)`

如果方法 A 反而更慢，再思考下有没更快的写法？

### DONE: 20251222-232501

## TODO:2025-12-22/3 将整数序列化方法改为 memcpy 拷贝缓存表的 2 字符

根据 `test::perf::WriteUnsignedCompare` 的测试结果显示，应该采用 memcpy 2 字符
更好些。据此修改 NumberWriter::WriteUnsigned 与 WriteSmall 的相关实现。

同时为简化代码，不必使用 `const DigitPair &pair` 中间变量了，
直接取 `const char* digit` 指针变量。

修改 wwjson.hpp 前，先备份 ./build-release/perf/pfwwjson 为 pfwwjson.last
修改后，再对比前后版本的性能测试，重点关注以下两个用例：
- `design_large_int` 
- `number_int_rel`

### DONE: 20251223-000925
