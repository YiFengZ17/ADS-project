# 按 20 分评分细则修订 Project 3

日期：2026-11-03。执行者：Claude（按用户要求修改）。修改前版本：`8abc454`。本记录是作者侧自查，不代替 YuJie Sun 的独立复现审批，也不是官方成绩。

环境：Linux 6.18，Intel Xeon 2.10 GHz（4 核），GCC 13.3.0，Python 3.13，TeX Live 2023（XeLaTeX）。

## 依据的评分框架

用户给出的 20 分框架：Item 1 封面与问题描述（1）、Item 2 算法（2）、Item 3 风格与提交包（1）、Item 4 测试（3）、Item 5 分析（3）、Item 6 程序（10，注释不充分可扣 10）。修改前估分约 15 分，主要风险是注释、性能表数据来自旧求解器、测试缺目的、提交包缺生成器。

## 修改内容

1. `code/main.cpp`：只改注释和代码排版，不改求解逻辑。增加文件头总体说明、结构体字段注释、方向常量 `UP/DOWN/LEFT/RIGHT`、`yes()/no()` 文字编码函数、关键行注释；拆开一行多句；把度数子句生成拆成 `add_degree_clauses`。新旧可执行文件在 1,700 个输入（随机小网格与 5～50 规模的构造/扰动网格）上的输出逐字节一致。
2. 新增 `code/gen.cpp` 生成器（planted / broken / random），进入提交包；`AGENT.md` 相应说明生成器属于提交源码。
3. `scripts/check.py` 新增 152 个细长网格（2×m、n×2，穷举对照，另含 2×50、50×2）和 50 个多解网格（穷举确认至少两解）。网格用例从 968 增至 1,170。
4. 新增 `scripts/benchmark_sizes.py`：用生成器在 10～50 规模、4 类输入、5 个种子上计时（预热 1 次 + 10 次取中位数），检查每个输出，生成 `figures/runtime-vs-size.png/.pdf`。
5. `report/report.md`：第 1 章补充难点与动机；第 2 章补充交叉示例、子句示例和主循环伪代码；第 3 章用单一环境重测，测试表增加 Purpose 列，旧求解器计时表与修订历史移出正文，换成当前程序的规模计时表和图；第 4 章新增 Discussion of Test Results，改进项改为删除学习子句和用二叉堆选变量；附录只引用提交包内的文件。封面日期保持用户此前设定的 2026-10-23。
6. 同步 TA、Peer Review 的源码、README 和 PDF；匿名 PDF 与 README 已检查不含姓名和组号。

## 验证

从 `recover-design/` 执行：

```bash
make check       # PASS: 1170 solver cases; 3 validator rejection checks; 2000 Boolean formulas; 4 search-path cases
make sanitize    # 同上，ASan/UBSan 无报错
make benchmark-sizes
make package-code
```

另按提交包 README 在 `TA/` 中逐条执行全部命令，样例、`pair` diff、sanitizer 和生成器均正常。PDF 经逐页渲染检查目录、表格、图和分页。

## 仍未完成

- 未取得 PTA 隐藏测试结果。
- 大规模无解输入只能由求解器本身判定，无法穷举确认；报告已写明。
- YuJie Sun 的独立复现审批仍待记录。
