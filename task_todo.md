# 项目开发原始需求管理

本文档由人工维护，**AI受限编辑** ，仅允许在每个 `## TODO` 下加个子标题 `###
DONE` 关联 `task_log.md` 的完成任务ID记录。

需求ID 格式：`TODO:YYYY-MM-DD/n` ，日期/编号。日期表示头脑风暴冒出该需求想法的
日期，若一天中产出多个想法则加编号。仅有初步概念但未细化的需求暂不加日期 id ，
先快速列出 `TODO: 标题`，后面有细化内容时再赋日期 id ，转为正式需求。

---

## 上个版本 v1.1.0 需求摘要

WWJSON v1.1.0 已完成发布，主要成果包括：

### 核心功能
- ✅ 新增 JString 高性能字符串缓冲类，专为 JSON 序列化优化
- ✅ BufferView 和 UnsafeBuffer 视图类，支持借用外部内存
- ✅ KString 最大不安全等级特化，单次内存分配零扩容
- ✅ wwjson::to_json 统一转换 API，简化结构体序列化
- ✅ 多头文件架构，jbuilder.hpp 组合使用 wwjson.hpp 和 jstring.hpp

### 性能优化
- ✅ UnsafePutChar 优化格式字符写入（逗号、冒号、引号）
- ✅ UnsafeConfig 优化字符串转义，避免临时 buffer
- ✅ 浮点数序列化优化，支持直接写入 StringBuffer
- ✅ JString 比 std::string 快 6-30%，KString 快 19-30%

### 开发工具
- ✅ 示例程序：struct_to_json、estimate_size、hex_json
- ✅ CI/CD 增强单元测试和性能测试流水线
- ✅ 多头文件安装到 wwjson/ 子目录
- ✅ 完整的文档更新和在线部署

### 详细记录
- [changelog/v1.1/task_todo.md](changelog/v1.1/task_todo.md)
- [changelog/v1.1/task_log.md](changelog/v1.1/task_log.md)

---

**后续需求记录**

## TODO: wwjson.hpp 优化整数序列化

优化 `NumberWriter::Output` 整数版，当 `unsafe_level<stringT>` 的值不小于 4 时，
直接从 stringT 末尾正向写入，避免逆向写入临时 buffer 。

整数序列化算法仍有优化空间：
- 探索正向写入算法，避免逆向写入临时 buffer
- 研究更优的数字对缓存表设计
- 对比 std::to_chars 的性能边界

## TODO: 浮点数序列化算法进一步优化

浮点数序列化：
- 提升 %.g 格式化性能，对比 yyjson 的 22 倍差距
- 研究 Ryū 等快速浮点数序列化算法
- 研究其他著名开源库的实现，如 yyjson fmtlib 等
