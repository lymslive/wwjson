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

## TODO:2025-12-02/3 重载 << 输入操作符支持链式拼装 json

希望 `>>` 作用于数组或对象时有不同含义，所以不好在 GenericBuilder 重载，而在两
个衍生类中重载。

- 添加数组元素：GenericArray << v1 << v2，链式调用 AddItem
- 添加对象字段：GenericObject << k1 << v1 << k2 << v2，交替调用 PutKey 与 AddItem

在 utest/t_operator.cpp 中添加测试用例覆盖新功能。

### DONE: 20251202-233040

## TODO: 支持 EndAll 方法

深层嵌套 json 末尾，可能是连续多个 `]` 或 `}` ，手动逐个调用 `EndArray` 与
`EndObject` 有点麻烦，提供 `EndAll` 方法一次性封端。
## UNDO

## TODO: 增加编译宏控制是否要额外维护栈

影响 << 重载与 EndAll 方法。
## UNDO
输入操作 << 不在 GenericBuilder 类重载，不用维护栈，保持简单

## TODO:2025-12-03/1 增加支持 std::string_view

现在 GenericBuilder 支持的字符串类型有 `const char*` 与 `std::string` ，以及内
部实现主方法用 `(const char*, size_t)` 参数表示字符串。需要再增加
`std::string_view` 参数重载，扩展以下两个基本方法：

- PutValue
- PutKey

然后其他地方调用 PutKey 的方法，也需要支持 `std::string_view` 。希望化简为模板
转发，先提取 `is_key` 的特性萃取，能匹配如下类：

- const char*: 应该也包括 char* 以及字符串字面量，
- std::string
- std::string_view

以下涉及调用 PutKey 的方法，改用 `is_key` 限定模板参数 `keyT`:
- GenericBuilder::AddMember
- GenericBuilder::AddMemberEscape
- GenericBuilder::operator[]
- GenericBuilder::BeginArray/BeginObject
- GenericBuilder::ScopeArray 与 GenericArray 构造方法
- GenericBuilder::ScopeObject 与 GenericObject 构造方法
- GenericObject::operator[]
- GenericObject::operator<<

再检查 utest 原来的单元测试用例，在涉及字符串类型或键处理的用例中，增加使用
`std::string_view` 的语句。

### DONE: 20251203-103249

## TODO:2025-12-03/2 wwjson.hpp 代码安全优化

分析 wwjson.hpp 中指针参数的使用，做必要的安全判空处理，提前返回。
尤其是 C-Style 字符串参数大量使用。

包括但可能不限于如下函数：
- PutKey
- PutValue
- AddItemEscape
- EscapeString
- EscapeKey

请全面分析是否还有其他安全隐患的代码，也一并修正。

### DONE: 20251203-171352
### DONE: 20251205~165145
补充 `wwjson_likely` / `wwjson_unlikely` 优化。

## TODO:2025-12-03/3 GenericBuilder 代码组织优化

M5 方法组只剩一个通用模板转发函数了，与 M4 合并，后续方法组命名编号递减。

PutNubmer 方法只有一处调用者了，无代码冗余问题，不必单独提取为函数。删除该函数
，其逻辑移回 AddItem(numberT) 方法。

同时删除 `t_custom.cpp` 文件中 `custom_putnumber` 测试用例，将
`custom_putnumber_quoted` 用例改名 `custom_number_quoted` 并调整其描叙，删除该
用例内对 PutNumber 的调用语句，只测试上层方法 AddItem/AddMember 在添加数字时的
不同行为。可增加对比情况，未定制 kQuoteNumber ，以及手动传第二参数 true 显式为
数字加引号。

### DONE: 20251203-173522

## TODO:2025-12-04/1 完善转义表

目标：在不增加代码复杂度、不影响性能的情况下尽量支持更多的字符转义，使组装的
json 是合法、可读的。

修改 BasicConfig::kEscapeTable：
- 不可打印 assic 字符尽量按标准转义
- 在转义表中按顺序重排
- 删除默认转义字符常量`DEFAULT_ESCAPE_CHARS`，无用的历史遗留了
- 在 `t_escape.cpp` 中添加单元测试覆盖

补充说明：
wwjson 为了性能与实现简洁，BasicConfig 只提供标准单字符转义，不转义 utf-8 ，也
不用 \u0000–\u001F 长转义。转义表补充 C/C++ 支持的标准转义，对于其中不符合
json 转义标准的，用尾注释备注说明。无标准转义的控制字符，统一转为 `\.` ，相当
于替换为点，类似 xxd 等工具打印二进制时用 `.` 代替不可打印字符。

允许用户自定义 configT ，实现完全的 json 转义标准，但 wwjson 暂不提供。

### DONE: 20251204-160535

## TODO:2025-12-04/2 GenericBuilder 支持添加 json 子串

应用场景：假设已有一个合法的 json 串，想把它加到另一个更大的 json 中，作为一个
数组的元素或对象的字段。比如最简单的 `{}` 空对象。

有两种处理方案：
- 当作一个字符串类型插入，一般需要转义子 json 中的引号。这用
  AddItemEscape/AddMemberEscape 可满足需求。
- 直接当成 json 源子串，保持其子结构，这在大部分场景下更合理。
  目前没有上层方法支持，只可用底层 append 实现。

所以需要增加一对类似的 AddItemSub/AddMemberSub 上层方法应对第二种处理需求。
这两个方法宜归于 GenericBuilder M8 方法组，属于处理子 json 的高级功能。

在 M3 方法组增加一个底层方法 PutSub ，类似 PutKey/PutValue 支持各种字符串参数
。实现上直接调用 Append ，不加额外引号，不转义。由用户负责传入的是合法子 json
串。M8 的 AddItemSub/AddMemberSub 尽可能简化为模板转调 PutSub 。

在 `t_advance.cpp` 中添加测试用例覆盖新功能。

### DONE: 20251204-164424

## TODO:2025-12-04/3 支持调用函数就地生成 json 子结构

GenericBuilder 组装嵌套 json ，目前支持的几种方法：
- 手动 BeginArray/Object 与 EndArray/Object 配对，End 方法一般还要传 true 参数
  补上逗号 `,` 才能正确地拼接下一个元素。
- 创建 ScopeArray/Object ，析构中自动调用配对的 End，可能需要在构造方法中传入
  true 参数补逗号 `,` 。
- 独立拼装子 json ，再用 Merge 方法，或 AddItemSub/AddMemberSub 方法合并。但这
  涉及额外拷贝，不宜有意这样做，只适合处理已有的子 json 串。

前两种方法本质上是一样的，ScopeArray/Object 理论上能使代码结构化更优雅，但有时
需要写一对裸 `{}` ，强行划分作用域使析构函数生效，有时看起来奇怪。另外嵌套较深
时适合将子 json 的组装逻辑提取到独立函数中。

为此，希望能在 AddItem/Member 方法中直接支持写个函数参数，调用函数就地组装子
json 。函数参数应该支持两种签名类型：

- 空参数，适合于 lambda 捕获当前 builder 引用
- 要求一个 GenericBuilder 引用参数，传入当前 builder 引用，适合独立函数

函数返回值可为 void ，一般在函数内直接操作当前 builder 。

实现要求：
- 原则上只要增加 AddItem 重载，AddMember 由通用模板方法转发自动支持函数
- 需要考虑与已有 AddItem 模板重载的兼容，能区分两种函数参数，各种数字参数，
  各种字符串参数，以及 bool 与 null 参数。
- 传函数参数的 AddItem 也按之前逻辑自动加逗号，函数只负责拼装合法 json

单元测试要求：
- 在 `t_advance.cpp` 新增用例覆盖新功能
- 测试场景包括调用 lambda，自由函数，类方法

再仔细思考该如何支持类方法更适合：
- 能为 AddItem 增加重载表达调用类方法吗？
- 让用户借用 lambda 封装，或 bind 适配后再传给 AddItem/AddMember

### DONE: 20251204-181600

## TODO:2025-12-04/4 重载 operator bool 支持 if 创建 Scope 变量

ScopeArray/Object 需要在 {} 作用域中使析构生效关闭相应括号，但是裸写 {} 比较奇
怪。可以写在 if 中，类似 `if (auto obj = builder.ScopeObject(); true) {}` 。为
了进一步简化写法，为相关类重载 operator bool 可省去 if 括号中的第二条件语句。

- GenericBuilder 的 bool 重载判断 !Empty()
- GenericArray/Object 的 bool 重载可固定返回 true, 因为构造函数会写个左开括号
  ，必定非空；可否标为 constexpr 函数

然后在 `t_scope.cpp` 增加测试用例，展示在 if 中使用局部变量创建嵌套子结构。

### DONE: 20251204-223557

## TODO:2025-12-05/1 修改 EndArray/Object 默认行为自动加逗号

当前实现：GenericBuilder End 方法允许接收一个可选 bool 参数，其实为了性能原因
减少判断是个重载版本，EndObject(true) 会多加一个逗号，而 EndObject() 不加逗号
，只加 `}'。

遭遇问题：在拼装嵌套 Json 时，经常忘记加 true 参数，使 EndArray 方法不能像
AddItem 方法一样自动加逗号，导致生成非法 json ，除非是父容器的最后一个元素，
两种 End 方法可不必加逗号。

一个可能的智能解决办法是每次 AddItem 或 BeginArray/Object 子结构时判断前面有没
逗号，但这会影响性能，每次在后面加逗号更快，只需在 EndArray/Object 时判断一次
尾逗号。

所以决定改变 EndArray/Object 默认行为，始终加上逗号，与 AddItem 逻辑保持一致。
唯一会导致的新问题时，在根数组或根对象结束时，也会加上逗号。需要使用 GetResult
方法处理最终的尾逗号，但也只需调用一次。

另一个原因是，在 json 最后多一个逗号的错误，比在中间少一个逗号更容易发现并纠正
。

具体修改方案：
- 删除 EndArray/Object(bool) 重载；在剩下的版本中加上 PutNext() 自动补逗号。
- GenericArray/Object 类删除 `m_next` 成员，相关构造函数与创建方法移除最后一个
  可选 bool 参数。析构函数简单调用 EndArray/Object.
- 增加 Begin/EndRoot(char) 方法，参数有默认值 `{` 或 `}` ，可传 `[` 或 `]'，但
  不必检查传入参数。EndRoot 不要加逗号。

这个默认行为的变动，会破坏原来的单元测试，修复方案：
- 先检查编译错误，删除调用 EndArray 等方法时额外传的 bool 参数；
- 再检查测试错误，如果是生成的 json 尾部多一个逗号，在 End 方法后多加一条
  GetResult 调用语句，或直接用 GetResult() 代替 builder.json 访问，这两种解决
  方法都要有体现。
- 注意不是所有测试用例都意图生成合法 json ，未报错的可不改。

然后在 `t_basic.cpp` 增加测试用例使用 BeginRoot 与 EndRoot:
- 在 `basic_builder` 第一个用例后面加一个 `basic_builder_root` 用例
- 在后面 `basic_builder_nest` 等一些用例中，将最外层的 BeginObject 改为
  BeginRoot ，但不必所有用例都改。

其他相关修改点：
- MoveResult 要调用 GetResult 保证正确处理了尾逗号
- AddItem 函数时，要判断函数本身没有加尾逗号时才加逗号

### DONE: 20251205-111355

## TODO:2025-12-05/2 建立性能测试框架

也用自研测试库 couttast 构建性能测试。

- 参考 utest/ 建立一个 perf/ 子目录
- perf/CMakeLists.txt 预计也用到 couttast 与 yyjson 库，因此将处理这两个依赖库
  的逻辑上移到根目录 CMakeLists.txt，xyjson 的依赖暂时只留在 utest
- 主 CMakeLists.txt 增加一个 WWJSON_LIB_ONLY 选项，默认 OFF。将 utest 与 perf
  子目录添加及其共用依赖的逻辑都放在 if NOT WWJSON_LIB_ONLY 限定之下。
- 主 CMakeLists.txt 增加 BUILD_PERF_TESTS 选项，默认 OFF ，需发主动打开选项才
  添加 perf 子目录
- 更新根目录 makefile 的封装命令，BUILD_PERF 同步改名 BUILD_PERF_TESTS

perf/ 子目录初步内容，先构建测试数据：
- readme 文档，初步的说明文档，后面再补充更新
- test_data.h/.cpp ，生成测试数据的函数集
- 编译可执行目标 pfwwjson，显式添加 test_data.cpp

perf/test_data.cpp 要求：
- 命名空间 test:: 内定义辅助函数
- 用 RawBuilder 生成 [0.5k, 1k, 10k, 100k, 500K, 1M] 几个不同规模的 json 。
- 接口函数可设计为一个，`void BuildJson(std::string& dst, double size)` ，size
  的单位是 k ，表示生成的 json 串预期大小。
- 参考 utest/ 的单元测试写法，在全局空间（test::之外）创建一个测试用例
  `data_sample`，调用 test::BuildJson 函数生成几个不同规模的 json ，校验其大小
  符合预期，误差不大。

用例 data_sample 补充要求：
- 使用 DEF_TOOL 代替 DEF_TAST ，这个宏的区别是在默认情况下不会运行，需在命令行
  显式指定参数才运行。
- COUT 断言语句，对于 double 参数，可传入第三参数表示允许误差，如
  `COUT(size*1.0, expect_size*1024.0, 10.0)`
- 将生成示例 json 写入 `perf/test_data.tmp/*.json` 文件。

### DONE: 20251205-143741

## TODO:2025-12-05/3 初始观察比较 wwjson 与 yyjson 构造 json 的性能

在 perf/test_data.cpp 中扩展生成函数，增加 test::yyjson::BuildJson() 函数，使
用 yyjson 的 api （可参考本地安装的 /usr/local/include/yyjson.h）生成与原来利
用 wwjson::GenericBuilder 生成完全一样的 json 串。更新 `data_sample` 用例，校
验大小符合预期误差，json 内容相同。

新增 perf/p_builder.cpp 文件，用 `DEF_TAST` 创建一系列测试用例，每个用例很简单
，就循环调用 `test::BuildJson` 或 `test::yyjson::BuildJson` n 次。运行
`pfwwjson` 会打印每个用例的运行时间（微秒单位）。

再增加 `perf/argv.h` 文件，处理命令行参数。
- 需要 include couttast/tastargv.hpp，已本地安装于 ~/include 目录
- 使用 `BIND_ARGV` 读取命令行参数的值
- 封装一个 test::CArgv struct ，含成员 int loop ，在构造函数中绑定命令参数
  `BIND_AGV(loop)` ，能获取命令行参数 --loop=n 的值。
- 在 perf/p_builder.cpp 定义的每个测试用例中，先定义一个 test::CArgv 对象，取
  其 loop 成员控制循环次数。
- 在测试用例循环调用 BuildJson 前后使用 `TIME_TIC` 与 `TIME_TOC` ，能更精确地
  控制用例运行计时，排除读取命令行参数的微小影响。循环后没有其他语句的话
  `TIME_TOC` 是没必要的。

最后运行 pfwwjson 数次，观察统计每个用例的运行用时。

另注：新 .cpp 文件需添加到 CMakeLists.txt

### DONE: 20251205-184030

补充通用用例 `builder_ex_wwjson` 与 `builder_ex_yyjson` ，由命令行参数控制构建
多大的 json ，循环几次，默认 `--loop=1000 --items=1000`

## TODO:2025-12-06/1 测试整数序列化性能

在 perf/test_data.cpp 文件中 `test::` 命名空间增加函数：

- BuildTinyIntArray(std::string& dst, uint8_t start, int count)
- BuildShortIntArray(std::string& dst, uint16_t start, int count)
- BuildIntArray(std::string& dst, uint32_t start, int count)
- BuildBigIntArray(std::string& dst, uint64_t start, int count)

构建 json 数组，交替写入正整数与负整数，例如传参数 (1, 3) ，输出
[1,-1,2,-2,3-,3] ，共 `2*count` 个元素。在小整数数组中，如果 count 比较大，从
整数溢界时返回 0 重新循环。

在 `test::yyjson::` 命名空间增加一组同名函数，用 yyjson api 构建相同的 json 数
组。

然后新增 perf/p_number.cpp 写几个测试用例，调用以上方法：
- array_int8_wwjson   array_int8_yyjson
- array_int16_wwjson  array_int16_yyjson
- array_int32_wwjson  array_int32_yyjson
- array_int64_wwjson  array_int64_yyjson

可参考 perf/p_builder.cpp 中 builder_ex_wwjson 与 builder_ex_yyjson 的写法。
读取 --start= 与 --items= 命令行参数分别传给 BuildIntArray 函数的 start 与 count 的参数。

### DONE: 20251206-103708

## TODO:2025-12-06/2 perf 子目录性能测试相关方法优化

- 将 BuildIntArray 等四个类似的用 wwjson builder 的创建方法改为用一个模板函数
，模板参数用 uintT ，代表原来支持的四个无符号整数。思考有没办法获取对应的有
符号整数类型。
- 再将通过的 BuildIntArray 函数放到 test::wwjson:: 命名空间，与 test::yyjson::
命名空间保持对称。
- 将 test:: 下面两个子空间的 BuildIntArray 函数都移到 `p_number.cpp` 文件中，
原来写在 test_data.h/.cpp 的那些同名函数都删除。
- test::yyjson::BuildIntArray 四个函数也应该能类似化简为一个模板函数，再修改一
下 api 调用，添加正整数时应该用 yyjson_mut_arr_add_uint，添加负整数时用
yyjson_mut_arr_add_sint ，不要都用 yyjson_mut_arr_add_int .
- test::wwjson::BuildIntArray 函数再增加一个可选参数，表示预估 json 大小，单位
k ，默认 1 。该参数乘 1024 后可传给 RawBuilder 的构造函数，预留 string 大小。
- p_number.cpp 中原来的 8 个 DEF_TAST 测试用例保持功能不变，但需要调整调用方法
，要保证调到对应的模板参数实例化函数。
- p_number.cpp 再增加一个 DEF_TOOL 定义的工具用例，用较小的 count 参数调用两个
被测的 BuildIntArray ，验证用 wwjson 与 yyjson 生成的 json 串完成一样。
- test_data.h/.cpp 中剩下的 test::BuildJson 也移到 test::wwjson 子空间中，带
double size 参数的版本，将 size*1024 传给 RawBuilder 构造函数，预留大小；带
int n (items) 参数的，也再增加可选一个预估的 size 参数，默认 1k ，传给
RawBuilder 构造函数。
- argv.h CArgv 增加 int size 成员，默认 1 ，绑定 --size 命令行参数；然后在
p_number.cpp 与 p_builder.cpp 中传给调用的 test::wwjson:: 相应 Build 函数。

### DONE: 20251206-234109

## TODO:2025-12-07/1 使用小整数缓存策略优化整数序列化

当前只用简单 std::to_string 实现整数序列化，需要改进性能。

整数序列化性能测试命令：`./build-release/perf/pfwwjson p_number.cpp` ，当前看
运行时间与 yyjson 相比有将近 10 倍的差距？

先备份 pfwwjson 可执行文件为 pfwwjson-1206 ，再优化代码，以便能对比老版本。
最终目标是希望接近或超过 yyjson 的序列化性能。

可参考及分析评估 doing_plan.tmp/small_int_optimization.cpp 文件中示范的小整数
优化方案。但要适配 wwjson.hpp 当前的编码风格。

基本实现要求：
- 在 StringConcet 后面 BasicConfig 之前增加一个模板类 NumberWriter<stringT>
- 对外接口是 Output(stringT& dst, intT value)
- BasicConfig 增加方法 NumberString 转发 NumberWriter::Output; 在
  GenericBuilder 中的 PutValue 整数时转发 BasicConfig::NumberString
- NumberWriter 的 0-99 整数表常量命名为 kDigitPairs，不要为显而易见的实现代码
  加注释，可以类与方法大元素加英文注释
- t_basic.cpp 增加一个单元测试全面测试序列化 8 种标准整数类型序列化结果正确，
  也要覆盖边界值。

小整数优化方案概要：
- 缓存 0-99 两位数字表，共 200 字符；
- 小于 10 的数字直接计算，10-99 的查表；
- 小于 10000 的小数字提前处理，正向写入目标字符串；
- 大于 10000 的大数字每次除 10000，反向写入临时栈缓存再一次性写入目标字符串；
- 模板处理各种整数类型，负数写完 `-` 后转正数处理；

### DONE: 20251207-114414

## TODO:2025-12-07/2 使用 std::to_chars 及回滚机制优化浮点数序列化

参考 doing_plan.tmp/to_chars_fallback.hpp

当前开发环境 gcc 版本支持 C++17 但支持 `std::to_chars` 不完整，不支持浮点数。
所以需要加个自动检测回滚降级机制。

需要适配当前 wwjson.hpp 代码风格，一些实现要求：
- 在 NumberWriter 类中增加 Output(stringT& dst, floatT value) 重载
- BasicConfig::NumberString 的模板参数加 is_arithmetic 限定，自动转发
  NumberWriter 的 Output 方法，
- GenericBuilder::PutValue(numberT nValue) 两个重载统一为一个

utest/ 单元测试修改：
- 原来的测试用例需要重新检查，因为原来 std::to_string 是固定 6 位小数，新方法
  是最短小数输出，用例中按字符串比较断言会失败，故需要调整期望值，移除多余的小
  数。
- t_basic.cpp 额外增加一个测试用例，专门测试浮点数各种情况下的序列化。

### DONE:20251207-184927

## TODO:2025-12-07/3 优化性能测试补充浮点测试

涉及修改的文件在 perf/ 子目录。

perf/p_build.cpp 修改：
- 构建不同大小 Json 的用例名风格改名，以 build_ 前缀，wwjson 或 yyjson 后缀，
  中间是预计大小；
- 在 loop==1 时打印 json 串内容（已纠正）

perf/test_data.cpp 的 test::yyjson::BuildJson 有个 bug ，可提供命令行参数
--loop=1 查看具体内容，输出的键名都是一样的，循环的最后一个键。请查阅 yyjson.h
api 使用正确函数，yyjson_mut_obj_add_arr/add_obj 的 key 参数可能是按引用的没有
复制各个键，可能要先以复制的方式创建键结点，再加入根对象。

perf/p_number.cpp 增加 BuildFloatArray 与 BuildDoubleArray 函数与相应的
DEF_TAST 测试用例。参考 BuildIntArray 的用例，也接收 --start --items 参数，对
于每个 item 整数，产生四个浮点数 +0.0 +1/5 +1/3 +1/2。

### DONE: 20251207-213311

## TODO:2025-12-07/4 增加 github Actions 流水线

- 在推送 tags 时触发
- 也可以手动触发，允许输入参数 start itmes loop size cases
- 编译 release 版
- 运行单元测试 utwwjson --cout=silent
- 运行性能测试 pfwwjson --start=? --items=? --loop=? --size=? cases
  手动触发若没有输入相关参数时，不要向 pfwwjson 传那个参数，保持程序默认值
  自动触发时不必传任何参数，按默认运行。

### DONE: 20251207-215828

## TODO:2025-12-08/1 数字序列化测试组织优化

数字序列化优化是比较关键的功能，可能需要更完善的测试，因此拟新建单独的测试文件
`t_number.cpp`。将 `utest/t_basic.cpp` 最后三个测试用例移到新文件，用例名前缀
也改成 `number_` 。

utest/t_experiment.cpp 新增两个测试工具用例，double_view 与 float_view, 接收一
个命令行参数 --value= ，研究这个浮点的二进制表示，打印其指数与尾数部分。可以参
考 perf/argv.h 用 BIND_ARGV 获取参数，但可以简单点绑定局部变量直接使用。如果没
有提供参数，就打印从 1/10, 1/9 到 1/2, 1/1这几个示例小数的二进制编码内容。可提
取一个函数分析浮点数二进制编码。

### DONE: 20251208~153630

## TODO:2025-12-08/2 设计字符串序列化性能测试

参考 `perf/p_number.cpp` 新增 `p_string.cpp` 文件，设计几组用例与构建函数没用
字符串的性能，命令行控制参数尽量复用已支持的参数。

- 构造字符串数组，将 [start, start+items) 范围的整数，先转成字符串，以字符串类
  型写入一个 json 数组。
- 构造字符串键值对，值与字符串数组用例一样，键名加一个 `k` 前缀，构建一个 Json
  对象。
- 构造含转义的 Json 对象，键名用 `k` 加数字前缀，值都用一个相同的子 json 串
  `{"key":"value"}` ，用 wwjson 构建时，显式调用 AddMemberEscape 。

小计三组用例，6 个函数，6 个用例。

### DONE: 20251208-174952

纯字符串序列化也比 yyjson 略快。那么混合 json 构建比 yyjson 慢的原因应该是浮点
数序列化了。

## TODO:2025-12-09/1 优化 CI 流水线与单元测试

.github/workflows/ci.yml 运行时报告了 7 个测试用例不通过：

!! basic_builder
!! basic_builder_nest
!! basic_low_level
!! scope_ctor_nest
!! scope_auto_nest
!! custom_builder
!! custom_scope

因为 CI 环境使用最新镜像已经支持 `std::to_chars` 序列化浮点数，这是期望行为。
那些用例用到 1/3 这个浮点数，被序列化为 `0.3333333333333333` 。

本来，由于浮点数误差，对浮点数进行全等断言并不合适，但是我们测试 json 构造，断
言整个串写起来也更方便。为了解决这个回归测试问题的矛盾，先将以上可能失败的用例
中使用 1/3 改为 1/4 ，使其在大部分算法能输出 `0.25` 。

然后将浮点数的专门测试集中在 `t_number.cpp` 文件，并且智能判断调整期望输出值。
增加一个简单测试用例 `number_std_support` ，检查观测当前运行时对 `std:: to_chars`
的支持度。使用标准 `std::cout` 或 `printf` 打印一行信息，否则用 `COUT` 或 `DESC`
宏不会在 `--cout=silent` 输出。

然后将 `ci.yml` 拆成两个流水线，分别运行单元测试与性能测试。触发条件支持两种：
- 自动触发，main push 且修改了 include/wwjson.hpp 或当前 yml 文件
- 手动触发，支持输入自定义命令行参数

测试测试与性能测试使用相同的框架，支持类似的命令行参数风格，在自动触发或未输入
参数时默认加上 `--cout=silent` 参数。自定义参数简化为一个输入框，允许输入多个
以空格分开的参数，需要原样传给 utwwjson 或 pfwwjson 程序，不要加引号当成一个参
数输入。并且 echo 实际将运行的完整命令行，以便检查运行参数。

### DONE: 20251209-101200

## TODO:2025-12-09/2 增加定点浮点数实验用例

utest/t_experiment.cpp
先将该文件中已有的函数封装在命名空间 `tool::` 中，调整用例调用处。

然后再写几个函数与用例，实现以下功能：
- 观察两位小数 f = [0/100, 1/100, ..., 99/100] ，用 `%g` 打印，每行 10 个
- 写个函数判断一个浮点数是否实际等于整数，并调用判断 100*f 是否整数，用 COUTF
  断言，期望是真，这个宏只在断言时有打印信息。
- 再打印一次 100*f ，一百个浮点数表，每行 10 个
- 再用 COUTF 断言每个 100.0*f/100.0 是否仍等于原 f 值
- 以上事务换 4 位小数再运行一次
- 以上事务换 8 位小数再运行一次
- 以上事务的浮点数类型希望既支持 double 也支持 float

所以尽量写成模板函数，模板参数 floatT ，分别传参数 100，10000, 10000*10000 。
共用 6 种情况，直接实现的话可以写 6 DEF_TOOL 用例入口。但建议写个通用入口用例
名，支持命令行参数 --type= --scale=，默认分别是 double 与 10000 。

float 的精度应该不支持 8 位小数，那会发生什么。
参数传 1000 与 10^8 时，打印浮点数表时只打印前 50 个与后 50 个，避免输出太多。

再写个用例，支持命令行参数 --value= 接收一个浮点数，默认用 double 存储，但也能
用 --type=float 指定用 float 表示。检查它有几位定点小数。先分离整数与小数部分
，再将小数部分每次乘 10 直到变成整数。并检查能否通过对整数与小数部分放大后的整
数再还原回原来的浮点数。

再写个用例，完整循环测试 [0, 9999.9999] 共 10000*10000 个浮点数，做类似上个用
例的事情所用的时间，但直接乘 10000 将小数部分转整数，再还原。粗略测时不用额外
操作，每个用例运行完会打印时间，只要在循环中不要有打印操作干扰计时。

### DONE: 20251209-145207

utwwjson fixed_point_generic
测试 10000 个四位小数，double 类型，
显示有 1149 个小数乘 10000 后不认为是整数，最大误差 9.0949470177292824e-13

utwwjson fixed_point_generic --type=float
失败率差不多，1144个，但最大误差 0.00048828125

## TODO:2025-12-09/3 优化小整数缓存表

NumberWriter::kDigitPairs 再提一层抽象，增加 DigitPair 结构体，只含 high low
两个 char 成员，而 kDigitPairs 是后者的数组，长度 100，总计仍是 200 字节。

然后在 WriteSmall 使用时，src 取 DigitPair 结构的值，而不是指针。分析确认一下
这样的改动是否能提升效率，因为指针 8 字节比这个结构体还大，并且是间接访问。

另外，输出整数的 Output 方法调用链上的其他方法也一致地加上 `is_integral_v` 限
定。

### DONE: 20251209-155416

## TODO: 优化 wwjson.hpp 英文注释

## TODO: 完善项目文档

新建两个文档：

- README.md 中文说明
- README-en.md 英文版翻译
- docs/usage.md 用户指南

README 只包含少量能展示项目特色的示例，usage 中需要详细、系统地介绍 wwjson 功
能、用法及配套示例。

## TODO: 同步文档示例与单元测试
