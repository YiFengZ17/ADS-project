# 作者侧完成记录与自查

日期：2026-11-25。执行者：Claude（按用户要求编写代码、运行实验并撰写报告；本机 git 身份为 YuJie Sun 使用的账号）。本记录是作者侧自查，不代替 YiFeng Zheng 的独立复现审批，也不是官方成绩。

环境：macOS 26.6，Apple M5，Apple clang 21.0.0，Python 3.9.6（matplotlib 3.9.4），TeX Live（XeLaTeX）。

## 依据

- 题面：8-1 Texture Packing（20 分细则），整理见 `problem.md`。
- 写法：根目录 `AGENT.md` 和 `STYLE.md`；目录、构建器和打包方式沿用 Project 5。
- 文献：[1] Coffman、Garey、Johnson、Tarjan 1980（NFDH、FFDH 及其界 $1.7\cdot OPT+h_{\max}$）；[2] Sleator 1980（$2\cdot OPT+h_{\max}/2\le2.5\cdot OPT$）；[3] CLRS 第 4 版第 2、13 章（排序与平衡搜索树）。[1][2] 的卷期页码与 DOI 经书目索引（DBLP、Scilit 等）核对，未打开出版社全文。报告第 2.5 节 $H<2A/W+h_{\max}$ 是自行推导的。

## 执行的命令与结果

从 `texture-packing/` 执行：

```bash
make check        # PASS：6 组 1,203 个用例，约 40 秒
make sanitize     # PASS：ASan/UBSan 下 217 个用例，无报错
make benchmark    # 两组实验约 30 秒；同一输入上 ffdh 与 ffdh-scan 结果一致
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

另把 `TA/` 复制到临时目录，逐条执行 `TA/README.md` 中的命令：样例 FFDH 首行 `6 5`、NFDH `7 5`、sanitizer 版 Sleator `6 5` 且无报错；`gen 10000 cut 1` 上 ffdh 和 sleator 的下界都是 `100000`。执行后提交目录没有多出文件。实名 PDF 15 页、匿名 PDF 14 页，逐页渲染检查了目录、表格、图和分页，LaTeX 无 Overfull；匿名 PDF 与 `Peer Review/README.md` 不含姓名、组号。

## 需要审阅者留意

- **题意理解。** 题面没有说能否旋转纹理，本项目按经典 strip packing 不旋转，报告 Introduction 与 Limitations 中已写明。
- **主算法选择。** 测试中 BFDH 的高度略优于 FFDH（144 个输入中 72 个更低、5 个更高，最多约 1%），但在 uniform / equalh / heavy 数据上慢 1.3～1.8 倍，且 FFDH 有论文证明的 1.7 界，故以 FFDH 为主算法。若审阅者认为应改用 BFDH，需相应调整报告第 2、4 章。
- **比值的含义。** 除 cut 类外，报告的比值是相对下界 $LB$ 的，只是真实近似比的上界；cut 类 $LB=OPT$，是真实比值。cut 类 $n=50$、种子 3 时 FFDH 达到 1.702，大于 1.7 但仍满足 $1.7\cdot OPT+h_{\max}$，报告 4.3 节已解释。
- **分工。** 按"两人轮换"假定本次由 YuJie Sun 编写、YiFeng Zheng 审批，需两人确认。

## 仍未完成

- YiFeng Zheng 的独立复现审批。

## 按 AGENT.md / STYLE.md 复查（同日）

逐条对照根目录 `AGENT.md` 和 `STYLE.md` 复查后修改：

- 报告：两张计时表合并为一张（STYLE "一个表加一张图"），质量表顺延为 Table 2；计时说明补"不含程序启动"；去掉 "kept" 这类暗示修改历史的说法；压缩两段过长的讨论；改正 "for ever"。
- 排版：`scripts/build_report.py` 让 "**Table N.**" 标题段与其后的表格保持同页，并给 "Packing Quality" 标题预留空间，避免标题孤悬页底。实名 15 页、匿名 14 页，无 Overfull，逐页渲染检查。
- 代码：`Rect`、`LevelTree` 构造函数、`Algorithm` 类型和两个 `main` 补注释；`gen.cpp` 文件头缩为 4 行；计时重复次数与生成器中的 100、10、10^6、10^9 等改为命名常量。生成数据与修改前逐字节相同。
- 脚本：`check.py`、`benchmark.py` 中缺注释的函数补一行说明。
- 文献 [2] 的作者名按 Crossref 记录（given "Daniel D.K.D.B."）保留为 "D. D. K. D. B. Sleator"；DBLP 与出版社页面本次未能打开核对。

复查后重新执行 `make check`（1,203 PASS）、`make sanitize`（217 PASS）、两份 PDF 构建、`make package-code`，并在 TA 副本中重跑 README 命令，输出与之前一致。报告中的计时数值未改动。
