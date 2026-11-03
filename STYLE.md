# ADS 报告与代码风格规范

以 Project 3（`recover-design/`，提交 `b8c285e`）的最终报告和 `code/main.cpp` 为范例。今后所有实验报告和代码注释都按本文件写；与 `AGENT.md` 中较早的文风描述冲突时，以本文件为准。

## 一、总体原则

- 写成认真的大二学生能写出的水平：内容完整、思路清楚，但不追求专业、工程化的表达。
- 用常用英文词和短句，多用主动语态。专业术语能用平白说法代替就不用（例如写 "follow the reasons back"，不写 "first UIP"）；必须用的术语，第一次出现时用一句话解释（例如 "A clause is a list of conditions joined by 'or'"）。
- 适量用 `we`，不要每句都以 `we` 开头。用 `first`、`then`、`so`、`because`、`however` 等连接词把步骤和原因串起来。
- 不虚构经历。写"我们一开始这样做、后来发现问题"时，必须是仓库历史或 `opinion/` 中有记录的真实过程。
- 不写平台名（如 PTA），用 "the problem"、"the judge" 等说法。

## 二、报告结构（对应 20 分评分框架）

| 章节 | 要写什么 | 对应评分 |
| --- | --- | --- |
| 封面 | 标题、完成日期（实名版另有作者、组号） | Item 1 |
| 1 Introduction | 用自己的话复述题意（不照抄题面），再用一段说明难在哪里、为什么需要这样的算法 | Item 1 |
| 2 Algorithm | 按模块分小节；每节先讲想法和原因，再讲做法；最后一节写正确性论证 | Item 2 |
| 3 Testing | 环境与方法、正确性测试表（含目的）、计时表和一张图 | Item 4 |
| 4 Analysis | 时间复杂度、空间复杂度、对测试结果的讨论、Limitations（最多 3 条） | Item 5 |
| 5 Conclusion | 一段总结 | — |
| References 及之后 | References、Author List、Declaration、Signatures、Source Code Appendix（匿名版省略身份相关章节） | Item 3 |

## 三、各章写法

### Introduction
- 第一段：输入是什么、规则是什么。第二段：输出格式。第三段：难点，最好带一个具体数字（例如 "almost 5,000 possible fences, so we can not try all $2^{5000}$ subsets"）。

### Algorithm
- 每个小节按"为什么需要 → 我们的想法 → 具体做法 → 一个小例子"展开，小例子只用在抽象规则上（例如 $d=3,k=2$ 时生成哪几条子句）。
- **不放伪代码块，也不放 ASCII 示意图。** 流程用 2～4 个编号步骤加粗小标题来写（如 **Propagation.** / **Guess.** / **Conflict.**），能用一句话说清的例子就不画图。
- 参数和细节只写影响理解的部分（例如重启间隔 "100 at first, growing by 1.5 times"），不展开实现技巧。
- 正确性论证两三段即可：模型与题目规则等价；剪枝不会删掉合法解；搜索结束时输出正确。引用标准结论时标 [n]。

### Testing
- 方法：一句话写环境（系统、CPU、编译器），一句话写编译选项和 sanitizer，一段说明检查器检查了什么。
- 正确性测试用三列表格：`Test group (count) | Result | Purpose`。Purpose 用一句话写这组测的是什么（最小规模、无解、多解、边界、最大规模等）。
- 大规模输入必须由提交包里的生成器（如 `code/gen.cpp`）产生，并在正文说明生成器有哪几类输入。
- 计时：所有数据在同一环境、用当前程序测得；每个输入预热 1 次再测 10 次取中位数；说明计时是否包含程序启动。一个表加一张图，表下用一两句说明单位、坐标和误差棒。
- 不放旧版本程序的数据，不写修改历史。

### Analysis
- 时间、空间复杂度只写关键推导，最坏情况如实写（例如 "the worst-case time is exponential"）。
- Discussion 用测得的数据解释"为什么这类输入快、那类输入慢"，每类输入一段，每段两三句。
- Limitations 最多 3 条，每条一两句：问题是什么，可以怎么改进。

## 四、代码风格

- **文件开头不写大段说明。** 只有生成器这类工具程序，开头可以用 3～4 行写用法。
- **每个函数或模块前写 1～2 行注释**，说明它做什么，用小写开头的简单英文：

  ```cpp
  // undo all values set after the given decision level
  void backtrack(int level) {
  ```

- **只给关键行加短注释**，说明这一行的意图，不解释语法：

  ```cpp
  if (boundaries.empty()) return false;  // conflict without any guess
  ```

- 结构体字段只在含义不明显时注释（如 `int da, db;  // direction of the edge seen from a and from b`）。
- 用命名常量代替魔法数字（如 `const int UP = 0, DOWN = 1, LEFT = 2, RIGHT = 3;`），小工具函数让代码自己说明含义（如 `yes(v)`、`no(v)`）。
- 一行只写一条语句，不写 `a; b; c;`；变量名能看出含义。
- 不用工程化的写法和词汇，注释数量以"读代码的人能看懂每个模块在做什么"为准：不能少到只有函数名，也不必每行都注释。

## 五、提交前自查

- [ ] 引言有题意复述和难点说明，没有照抄题面
- [ ] 算法章节每节先讲原因；有正确性论证；没有伪代码块和 ASCII 图
- [ ] 测试表每组都有 Purpose；有最小规模、无解、多解和接近最大规模的输入
- [ ] 生成器在 `code/` 里，报告和 README 写了用法
- [ ] 计时数据来自当前程序和同一环境，有表有图
- [ ] 复杂度、结果讨论齐全；Limitations 不超过 3 条
- [ ] 每个函数都有简短注释，关键行有注释，没有大段文件头
- [ ] 正文没有平台名、过时数据和修改历史
