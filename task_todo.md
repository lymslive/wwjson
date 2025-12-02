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

## TODO:2025-11-25/2 调整单元测试及命令配置

- 将测试 `std::to_chars` 的用例移到 `t_experiment.cpp` 新文件
- 迁移 xyjson 项目根目录的 makefile 配置开发命令，调整文件名路径
- 根目录创建几个符号链接
  + ln -s build/compile_commands.json 
  + ln -s build/utest/utwwjson 

`std::to_chars` 在当前 GCC 9.4.0 只支持整数，不支持浮点数。

### DONE: 20251125~145323

## TODO:2025-11-25/3 增加 scope 方法创建能自动关闭的数组或对象

RawJsonBuilder 增加两个方法：
- ScopeArray 创建 RawJsonArray，允许参数类似 BeginArray
- ScopeObject 创建 RawJsonObject，允许参数类似 BeginObject

这样，用户只需要记一个类名 RawJsonBuilder ，另两个衍生类可以用方法创建而用
auto 接收。

衍生类 RawJsonArray/RawJsonObject 其他优化：
- 分析成员 `m_build` 的类型用指针还是引用更好？
- `m_build` 与 `m_next` 改为 private 权限
- `m_build` 改成 `m_builder`

再优化相关单元测试：
- 增加 `t_scope.cpp` 文件，将原来的 `basic_builder_nest` 移到这个新文件并改名
  `scope_builder_nest`
- 再增加一个测试用例，改用 auto 与 Scope 方法创建衍生类而不是构造函数

### DONE: 20251125-162627

## TODO:2025-11-25/4 模板化重构

当前 json 序列化目标类型是 `std::string`，希望泛型化也支持其他库的自定义字符串
类型，只要与 `std::string` 具有相同接口。

- RawJsonBuilder 改名 GenericBuilder<stringT>
- 增加别名 typedef GenericBuilder<std::string> RawBuilder
- RawJsonArray/RawJsonObject 也要相应改名 GenericArray/GenericObject，
  并简化别名 RawArray/RawObject
- 也需要处理模板类的前向声名
- 单元测试的使用的类名同步修改为 typedef 后的简化别名 ，应该保持功能不变

事实上我们只用到 `std::string` 的部分接口，定义一个 `struct StringConcept` ，
列出 GenericBuilder 需要用到的接口。虽然 C++17 还不支持 concept 限定，但定义一
个 struct 作为文档说明也有意义。

### DONE: 20251125-172718

## TODO:2025-11-26/1 测试使用自定义字符串类型构建 json

- 增加 utest/custom_string.cpp(.h) 实现一个简单的自定义字符串类 test::string，
  满足 wwjson::StringConcept 说明的那个接口。
- 增加 utest/t_custom.cpp 测试文件，写几个测试用例，以 test::string 代替 std::string
  构造 json 串。
- 同步修改 utest/CMakeLists.txt 通过编译测试

### DONE: 20251126-114254

## TODO:2025-11-26/2 配置化重构

GenericBuilder 再增加一个模板类型参数 `configT<stringT>`，用于定义序列化选项：
- 对象键与字符串类型的值，要不要以及如何转义
- 数字类型的值要不要加引号统一转为字符串
- 对象与数组是否允许尾逗号（允许尾逗号时实现更简单）

暂不打算支持格式化美化缩进的方式输出，因为库的设计目的是高性能构建 json 串，不
输出额外没必要的信息。

定义一个 BasicConfig 类，只有静态方法与编译期常量，允许用户继承或参考它重写自
定义的配置选项。

BasicConfig 大约有以下成员：
- EscapeKey(stringT& dst, ...) 方法，实际不必转义，直接拷贝；
- EscapeValue(stringT& dst, ...) 方法，只做基本转义，转调 EscapeString；
- EscapeString(stringT& dst, ...) 方法，将现有的 GenericBuilder::EscacpeString
  逻辑移至 BasicConfig 类，删去 GenericBuilder 的原方法；
- bool kAlwaysEscape = false; 只有显式调用 AddMemberEscape 变体方法才需要转义
  ，为 true 时，AddMember 方法也默认需要转义；
- bool kQuoteNumber = false; 默认不将数字转为字符串，但 AddItem/AddObject 仍可
  通过可选参数显式要求转字符串；
- bool kTailComma = false; 现有逻辑不允许尾逗号，在 EndArray/EndObject 中需要
  额外判断 back() 字符是否逗号，为 true 时可避免这个判断。

补充一致性逻辑：
- 增加 AddItemEscape 变化方法，类似 AddMemberEsacpe 显式转义字符串值

由于各个 Escape 方法要直接原位写字符串，BasicConfig 本身也该是模板类。
GenericBuilder 的第二参数可以有默认值 `BasicConfig<stringT>` 。

其他考虑：我觉得 config 的命名可能不是很准确，如果你有更合适的命名提议，可改名
。

### DONE: 20251126-161114

## TODO:2025-11-27/1 完善头文件注释文档

完善 wwjson.hpp 的注释文档。简洁风格，每个元素的注释以 /// 开始，第一句要用
`.` 结束。每个方法原则上只有一行一句，复杂的允许有多句。

仅文件头的注释风格可保持，需补充 detail.

### DONE:20251127~094126

by vscode plugin.
不尽如人意。还有手工校对。

## TODO:2025-11-27/2 字符串与转义功能优化

- 删去 BasicConfig::EscapeValue ，就让 Builder 直接使用 EscapeString
- BasicConfig::EscapeString 最后一个参数是 char 的版本不提供默认值，
  const char* 版本提供默认值 "\\\n\t\r\"\0"
- EscapeKey 也参考 EscapeString 写两个重载版本
- GenericBuilder::PutKey 方法要判断 configT::kAlwaysEscape
- GenericBuilder::PutValue 的字符串重载也要判断 configT::kAlwaysEscape
- AddMemberEscape/AddItemEscape 的重载参数 char 与 const char* 的默认值规则调
  换，要与 BasiceConfig::EscapeString 一致；该默认值可定义一个常量避免重复。
- GenericArray 增加 AddItemEscape 转发
- GenericObject 增加 AddMemberEscape 转发
- 检查所有添加字符串值的方法，包括 AddItem/AddMember 及其 Escape 变体，字符串
  参数应该支持如下四种重载(目前 AddItemEscape 最丰富)：
  + const char*
  + const char*, size_t
  + const std::string&
- 新增 utest/t_escape.cpp 测试文件，补充更全面的字符串与转义测试用例

另注： 添加字符串方法不要加 `stringT&` 参数重载。在常用情况下，模板参数就是
`std::string` ，不能同时定义会出现二义性。`stringT` 主要用于拼接目标
string(buffer) 类定义，输入字符串先只支持标准类。

### DONE: 20251127-112133

## TODO:2025-11-27/3 重构 GenericBuilder 方法封装及顺序调整

增加封装方法，隔离依赖 stringT 的接口在少量方法内：

- 增加 PutChar 方法统一入口调用 stringT::push_back 方法
- 删除 PutComma ，SetItme 改为 PutNext 调用 PutChar(',')
- 增加 FixTail 方法，EndArray 默认分支调用 FixTail(',', ']')，EndOject 类似
- 增加 Append 方法，调用 stringT::append 方法
- 其他方法不再显式调用 json.push_back/append 等

方法数量比较多了，按以下分组调整顺序：

- M0: 基本方法，目前只有构造方法
- M1: 对接 stringT 接口的封装/桥接方法，PutChar/FixTail/Append 等
- M2: 处理 json 单字符拼接的方法，PutNext, Begin/End Array/Object 等
- M3: 处理 json 标量值的方法，PutNull/PutKey/PutValue 等
- M4: 添加 json 数组元素的方法，AddItem 等
- M5: 添加 json 对象元素的方法，AddMember 等
- M6: 处理字符串转义的方法变体，*Escape
- M7: 创建 ScopeArray/Object 的方法
- M8: 其他高级方法，Merge 等，其中用到的 size/back/front 方法也提到 M1 ，命名
  首字母改大写。

每个分组加上显著注释与分隔，格式形如：

/// M0: title
/* -------- */ (整行总长 78 字符)

补充说明：
- EmptyArry/EmptyObject 暂放在 M2 后面，相当于连续调用 Begin/End 方法
- EndLine 也放在 M2 后面
- 再检查核对 M0 M1 组中的方法调用了 stringT 哪些接口，与 StringConcept 保持一
  致，不多不少。且取消注释，直接写出完整签名的声明。原则上 M2 组以后的方法不要
  直接调用 stringT 的方法。

单元测试，没有新功能，没改变逻辑。补充用例：
- `t_basic.cpp` 中补充对 M1 各方法的简单测试
- `t_cusom.cpp` 中也补充一个类似的用例，验证 M1 的接口方法可定制

### DONE: 20251127-14162

## TODO:2025-11-27/4 优化 GenericBuilder Meger Json 功能

目标：include/wwjson.hpp GenericBuilder M8 分组下的 3 个方法。

当前实现只能处理根是对象 {} 的 json ，希望能扩展支持根是数组 [] 的 json .

- ReopenOject 方法名简化为 Reopen
- static MergeObject 方法简化为 Merge
- 三个方法都返回 bool
- Merge 时只有同为对象为同为数组时才能 Merge ，否则返回 false 表示失败
- 单元测试增加 `utest/t_advance.cpp` 文件，补充测试用例

### DONE: 20251127-171004

## TODO:2025-11-27/5 优化 GenericBuilder 方法间调用

分析 PutValue/AddItem/AddMember 三套方法及其各自的重载版本，尽量简化、复用代码
。

我初步发现的一些问题与修改建议：
- PutValue 三种字符串重载版简化以 (const char*, size_t) 为主
- AddItem/AddMember 三种字符串版本重载可合并为模板方法
- *Escape 方法也以 `(const char*, size_t)` 参数为主版本

请再深度分析有没其他可优化点。

### DONE: 20251127-180345

## TODO:2025-11-28/1 完善添加特殊 json null/boll/空容器的支持访求

- PutValue 增加 std::nullptr_t ，保用高层方法 AddItem/Member 支持增加 null
- `t_basic.cpp` 补充用例测试构造 json 时添加一些 null bool 空数组、空对象。
- 增加测试用例只用低层 PutKey/PutValue/PutNext 构造 json

### DONE:20251128-100515

## TODO:2025-11-28/2 GenericBuilder 方法再优化

- 提取 PutNubmer 方法统一判断是否自动加引号，AddItem/AddMember 添加数字类型改调
PutNumber ，手动加引号的版本不改。
- 提取 EscapeString 方法，字符串参数三种重载，escape 目标字符两种重载，共 6 个
  方法。但原来 AddItemEscape/AddMemberEscape 的 6 个方法应该能化简一个模板方法。
- 在 `t_custom.cpp` 增加测试用例覆盖 PutNumber ，以及自定义 configT::kQuoteNumber
  的情况。
- 在 `t_escape.cpp` 增加测试用例覆盖 EscapeString 方法。

### Rollback:

这样提取 EscapeString 6 个重载看来确实会出现二义性。先把对 EscapeString 的改动
回滚。保留 PutNumber 但我希望放在 PutKey 函数后面。然后再分析一下 EscapeString
还有没优化空间。这个方法设计的原由是因为转义字符串比较慢，因此推荐默认配置不转
义，但用户可以根据业务需求只转义某一个字段的值，然后进一步如果用户知道可能出现
的特殊字符，还可以当作将具体待转义字符传参进去，所以出现了这么多重载方法。

### DONE: 20251128-113745

## TODO:2025-11-28/3 使用转义表重构 EscapeString 方法

目标：inclue/wwjson.hpp 的类 BasicConfig 与 GenericBuilder

参考 doing_plan.tmp/escape_table_implementation.md 文档中对 BasicConfig 的建议
，增加编译期转义表，用一个 EscapeString 方法代替原来的两个重载，减少一个参数传
递，只由转义表决定如何转义。

接口涉及不兼容修改，需要同步修改 GenericBuilder 与单元测试：
- `basic_escape` 用例删除，让转义测试用例集中在 `t_escape.cpp`
- `t_escape.cpp` 的用例可以重新设计，覆盖修改后的转义功能实现
- 其他测试文件的用例应该不受影响

### DONE: 20251128-163723

## TODO:2025-11-28/4 扩展 PutKey 支持三种字符串重载

GenericBuilder::PutKey 目前只支持一种 (const char*) 类型的字符串参数，需要与
PutValue 字符串参数支持一致，以 (const char*, size_t) 为主方法，另两个重载调用
它。

BasicConfig::kAlwaysEscape 配置项拆成 kEscapeKey 与 kEscapeValue 两个常量，默
认都是 false, PutValue 判断 kEscapeValue ，PutKey 判断 kEscapeKey ，分别控制。
检查 `t_escape.cpp` 用例，同步这项修改，相当于原来只测了 kEscapeValue, 增加
kEscapeKey 的配置测试。

最后再分析一下 AddMember(Escape) 上层 api ，如果说 key/value 都支持三种字符串
重载，那组合情况就有 9 种，方法重载数量暴增的问题如何解决，以及这 9 种重载都写
来的话是否会可能有二义性。

最接近目标的解决方案(2/3)：
AddMember 改用不定模板参数实现为 PutKey + forward AddItem.
但是不定参数包之前只能允许一个 Key 参数，const char* 或 std::string&.
AddMemberEscape 类似.

### DONE: 20251128-180546

## TODO:2025-12-01/1 增加 GetResult/MoveResult 方法

目标：inclue/wwjson.hpp 的类 GenericBuilder，在 M0 方法组中增加新方法

- GetResult() const 返回构建好的 json
- GetResult() 非 const 版本，检查构建的 json 末尾是否残余尾逗号，删除之再返回
  json ，可解决 EndArray/Object 意外添加的逗号
- MoveResult() 返回右值引用的 json，转移构建好的 json 串

然后在 `utest/t_basic.cpp` 增加测试用例覆盖新功能。

### DONE: 20251202-000356

## TODO:2025-12-02/1 为 GenericBuilder 重载 [] 索引与赋值操作符

希望实现如下功能：
- builder["key"] = value; 添加对象字段，key 支持 `const char*` 与 `std::string`
- builder[-1] = value; 添加数组元素，支持任意整数 size_t

判断一下 builder[0] 是否会出现编译歧义问题。整数参数只定义 `size_t` 的话，传入
int 是否会自动提升？

我的初步方案：
- operator[] 返回 *this，[key] 先调用 PutKey
- operator= 调用 AddItem，支持各种单参数泛型
- 特化类本身的拷贝赋值函数与移动赋值函数
- GenericArray/Object 也要支持 operator[]

这几个操作符重载定义的位置放在 wwjson.hpp GenericBuilder 的 M7 方法组，原来的
M7 M8 方法组后移为 M8 M9。

然后为了对称，在 M0 构造函数后面，增加拷贝构造与移动构造函数。

utest/ 子目录增加 `t_operator.cpp` 文件测试这些新功能。

### DONE: 20251202-111929

## TODO:2025-12-02/2 单元测试引入三方库校验构造 json 合法性

在 `utest/test_util.cpp` 已经写了个简单校验函数 `test::IsJsonValid`，它依赖
xyjson.h ，而后者依赖 yyjson.h 库。请完善 CMake 构建脚本，使该函数可用。参考依
赖 couttast 库，先从本地查找，再从远程 github 下载。

当前本地安装情况：
- yyjson 安装在系统目录 /usr/local
- xyjson 安装个人 $HOME 目录，且在 ../xyjson 兄弟目录有源码

然后分析其他单元测试文件，使用 `test::IsJsonValid` 校验拼装的 json 是合法的。
注意有些测试用例测试中间结果，未必都是合法的 json 。需要找出意图拼装完整的
json 的用例，断言其是合法有效的 json 。

### DONE: 20251202-213730

## TODO: 重载 << 输入操作符

添加数组元素：json << v1 << v2
添加对象字段：json << k1 << v1 << k2 << v2

可能需要加一个栈，保存当前（尾部）操作的是数组还是对象。

## TODO: 支持 EndAll 方法

深层嵌套 json 末尾，可能是连续多个 `]` 或 `}` ，手动逐个调用 `EndArray` 与
`EndObject` 有点麻烦，提供 `EndAll` 方法一次性封端。

## TODO: 增加编译宏控制是否要额外维护栈

影响 << 重载与 EndAll 方法。

## TODO: 优化 std::to_string 性能

对标 `std::to_chars` 的性能。
有些 gcc 版本支持 C++17 但支持 `std::to_chars` 不完整。
