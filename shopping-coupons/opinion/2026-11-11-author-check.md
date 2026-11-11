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

另按 `TA/README.md` 在提交目录中逐条执行编译、样例、sanitizer、生成器和计时命令，输出正常。实名和匿名 PDF 均为 11 页，逐页渲染检查了目录、表格、图和分页；匿名 PDF 与 `Peer Review/README.md` 不含姓名、组号。生成器另用 Clang + libc++ 编译，与 GCC 版本的输出逐字节一致。

## 需要审阅者留意

- **题意理解。** 题面没有明说能否不用优惠券按原价购买。本项目按样例理解为每次购买都用一张券（最多 N² 次购买）。手工用例"one pair, each pair only once"（N=1、D=100，答案 `1 94`）体现了这一理解。尚未提交在线评测验证；如评测不通过，优先检查这一点。
- 实验 2 中 D = 10^7 超出题目上限（10^6），只用来看趋势，报告中已注明。
- equal 类型三个种子生成的数据相同（所有价格、券面值都一样），其误差棒只反映计时波动。
- 报告封面的分工（YiFeng Zheng 编写、YuJie Sun 审批）是按"本次由 YiFeng Zheng 发起"和角色互换的约定推定的，如不符请告知后修改。

## 仍未完成

- YuJie Sun 的独立复现审批。
- 在线评测的提交结果。
