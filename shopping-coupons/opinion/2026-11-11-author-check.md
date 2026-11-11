# 作者侧完成记录与自查

日期：2026-11-11。执行者：Claude（按用户 YiFeng Zheng 要求编写代码、运行实验并撰写报告）。本记录是作者侧自查，不代替 YuJie Sun 的独立复现审批，也不是官方成绩。

环境：Linux 6.18（x86-64），Intel Xeon 2.10 GHz（4 核），GCC 13.3.0，Python 3.13，TeX Live 2023（XeLaTeX）。

## 依据

- 题面：8-1 Shopping With Coupons（20 分细则），整理见 `problem.md`。
- 写法：根目录 `AGENT.md` 和 `STYLE.md`；目录、构建器和打包方式沿用 Project 4。
- 文献：只引用《Introduction to Algorithms》第 4 版第 6 章（堆操作的 $O(\log n)$），书目沿用 Project 2 已核对的条目。贪心规则的正确性是自行推导的，写在报告 Correctness 一节。

## 执行的命令与结果

从 `shopping-coupons/` 执行：

```bash
make check        # PASS：7 组 1,639 个用例
make sanitize     # PASS：ASan/UBSan 下 354 个用例，无报错
make benchmark    # 两组实验约 2 分钟；所有输入上各方法结果一致
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

另按 `TA/README.md` 在提交目录中逐条执行编译、样例、sanitizer、生成器和计时命令，输出正常。实名和匿名 PDF 均为 10 页，逐页渲染检查了目录、表格、图和分页；匿名 PDF 与 `Peer Review/README.md` 不含姓名、组号。生成器另用 Clang + libc++ 编译，与 GCC 版本的输出逐字节一致。

## 需要审阅者留意

- **题意理解。** 题面没有明说能否不用优惠券按原价购买。本项目按样例的做法，每次购买都用一张券（最多 N² 次购买）；YiFeng Zheng 已确认按此实现，并决定不把这一点作为假设写进报告。手工用例"one pair, each pair only once"（N=1、D=100，答案 `1 94`）体现了这一做法。尚未提交在线评测验证；如评测不通过，优先检查这一点。
- 实验 2 中 D = 10^7 超出题目上限（10^6），只用来看趋势，报告中已注明。
- equal 类型三个种子生成的数据相同（所有价格、券面值都一样），其误差棒只反映计时波动。
- 分工已由 YiFeng Zheng 确认：YiFeng Zheng 编写，YuJie Sun 审批。计时数据采用本记录所述环境。

## 仍未完成

- YuJie Sun 的独立复现审批。
- 在线评测的提交结果。

## 同日修订：按用户意见简化报告

YiFeng Zheng 认为初版报告偏专业、偏细，要求写得更像大二学生的手笔。本轮只改报告文字和一处代码注释，求解逻辑与测试数据不变：

- Introduction 改为直接用题面样例讲清题意，再说明难点；
- 算法一章去掉花费表格，用文字说明"每件商品的组合从便宜到贵排成一列"；"Threshold method"改名为"Binary search method"（图例、提交 README 和 `main.cpp` 的一行注释同步改名）；
- Correctness 改为说明大意，不追求严格证明；测试表的 Purpose、复杂度和结果讨论都改短；
- 报告由 11 页减到 10 页；重新运行 `make check`、`make sanitize`、`make package-code` 并重建两份 PDF。

