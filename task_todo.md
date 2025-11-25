# 项目开发原始需求管理

本文档由人工维护，**AI受限编辑** ，仅允许在每个 `## TODO` 下加个子标题 `###
DONE` 关联 `task_log.md` 的完成任务ID记录。

需求ID 格式：`TODO:YYYY-MM-DD/n` ，日期/编号。日期表示头脑风暴冒出该需求想法的
日期，若一天中产出多个想法则加编号。仅有初步概念但未细化的需求暂不加日期 id ，
先快速列出 `TODO: 标题`，后面有细化内容时再赋日期 id ，转为正式需求。

## TODO:2025-11-25/1 完成 cmake 构建系统能编译、运行单元测试

wwjson 预期作为单头文件库使用，原则上不需构建直接引用头文件。但希望符合 C++
cmake 系统规范，也该提供安装功能，方便其他项目使用 cmake 集成。

单元测试的源码与构建脚本都放在 utest/ 子目录下。依赖自研的 `lymslive/couttast`
单元测试框架库，优先在本地 `find_package` (支持在个人 HOME 目录查找)，本地未安
装再从 github 下载依赖。

单元测试的编译可执行目录为 utwwjson 。根目录的 CMake 脚本添加选项是否编译单元
测试，默认是。

### DONE: 20251125-112235

## TODO:2025-11-25/2 调整单元测试及命令

- 将测试 `std::to_chars` 的用例移到 `t_experiment.cpp` 新文件
- 迁移 xyjson 项目根目录的 makefile 配置开发命令，调整文件名路径
- 根目录创建几个符号链接
  + ln -s build/compile_commands.json 
  + ln -s build/utest/utwwjson 

`std::to_chars` 在当前 GCC 9.4.0 只支持整数，不支持浮点数。

### DONE: 20251125~145323


