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

## TODO: 初始观察比较 wwjson 与 yyjson 构造 json 的性能

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

## TODO: 测试整数序列化性能

在 perf/test_data.cpp 文件中 `test::` 命令空间增加函数：

- BuildTinyIntArray(std::string& dst, uint8_t start, int count)
- BuildShortIntArray(std::string& dst, uint16_t start, int count)
- BuildIntArray(std::string& dst, uint32_t start, int count)
- BuildBigIntArray(std::string& dst, uint64_t start, int count)

构建 json 数组，交替写入正整数与负整数，例如传参数 (1, 3) ，输出
[1,-1,2,-2,3-,3] ，共 `2*count` 个元素。在小整数数组中，如果 count 比较大，从
整数溢界时返回 0 重新循环。

新增 perf/p_int.cpp 写几个测试用例，调用以上方法。

## TODO: 使用小整数缓存策略优化整数序列化

参考 doing_plan.tmp/small_int_optimization.cpp

## TODO: 使用 std::to_chars 及回滚机制优化浮点数序列化

参考 doing_plan.tmp/to_chars_fallback.hpp

当前开发环境 gcc 版本支持 C++17 但支持 `std::to_chars` 不完整，不支持浮点数。

## TODO: 优化 wwjson.hpp 英文注释

## TODO: 完善项目文档

新建两个文档：

- README.md 中文说明
- README-en.md 英文版翻译
- docs/usage.md 用户指南

README 只包含少量能展示项目特色的示例，usage 中需要详细、系统地介绍 wwjson 功
能、用法及配套示例。

## TODO: 同步文档示例与单元测试
