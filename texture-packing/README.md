# Project 6 — Texture Packing

Group 17。编写：YuJie Sun（YujieSun）；审批：YiFeng Zheng（YifengZheng）。实现为 C++17：主算法 FFDH（First-Fit Decreasing-Height），用一棵记录各层剩余宽度最大值的线段树找"最低的放得下的层"，$O(n\log n)$；Bonus 对比 NFDH、BFDH 和 Sleator 2.5 近似算法，另保留逐层扫描版 FFDH 作计时对照。题面整理和本项目对题意的理解见 [problem.md](problem.md)。

## 文件

- `code/main.cpp`：全部算法和计时模式。`./pack < input` 用 FFDH 输出 `高度 下界` 和每个纹理的左下角；`./pack ffdh|ffdh-scan|nfdh|bfdh|sleator < input` 指定算法；`./pack ALGORITHM RUNS < input` 为计时模式。
- `code/gen.cpp`：测试数据生成器（uniform / equalh / heavy / cut 四类，strip 宽度固定 1000；cut 类最优高度恰为 10n），任何编译器生成同一份数据。
- `scripts/check.py`：正确性测试（手工用例、随机小用例与 Python 重写版逐坐标对照、生成数据、大规模输入、非法输入、生成器可移植性），并检查无重叠、高度、下界及论文中的近似界。
- `scripts/benchmark.py`：两组实验（规模 × 数据类型的计时与质量；n = 1000 时最大宽度的影响），数据写入 `data/*.json`，图写入 `figures/`。
- `tests/sample.in`：手工小样例（FFDH 高度 6，NFDH 高度 7，下界 5）。
- `figures/`：报告使用的四张图，提交 Git。
- `report/report.md`：报告正文的唯一权威来源。
- `scripts/build_report.py`、`scripts/report-style.tex`：沿用 Project 5 的构建器和仓库模板样式，支持单独生成匿名版。
- `TA/`：实名报告 `yds_2026G17_P6.pdf`、`code/` 和 `README.md`。
- `Peer Review/`：匿名报告 `texture-packing.pdf`、`code/` 和 `README.md`。
- `opinion/`：复现记录和审批意见。
- `problem.md`：题面整理，只留在开发目录。
- `build/`、`data/`：本地输出，不提交。

## 编译、验证和计时

以下命令从 `texture-packing/` 执行，需要 GCC 或 Clang（C++17）、make 和 Python 3；测试脚本仅使用标准库，画图需要 matplotlib。

```bash
make
./build/pack < tests/sample.in
make check
make sanitize
make benchmark
python3 scripts/benchmark.py --plot-only
```

`make` 输出 `build/pack` 和 `build/gen`。样例第一行应为 `6 5`。`check` 在终端报告 6 组共 1,203 个用例（约 40 秒）；`sanitize` 输出 `build/pack-san`（ASan/UBSan），并运行其中 217 个较小用例。

`benchmark` 是报告表格和图的来源：`size`（4 类数据 × n 从 10 到 10^5 共 12 档 × 3 个种子 × 5 个算法，逐层扫描版只测到 3×10^4）和 `width`（n = 1000 的 uniform 数据，w_max 从 50 到 1000，10 个种子，只看质量）。每个输入预热 1 次、正式运行 10 次取中位数，短于 1 ms 的运行重复到 1 ms 再取平均。输入文件缓存在 `data/instances/`，结果写入 `data/size.json`、`data/width.json`，图写入 `figures/{time,ratio,width,example}.{png,pdf}`，终端同时打印报告用的表格数据。全部实验约需 30 秒。`--plot-only` 只从已有 JSON 重画图和表。新运行不会改写报告中的数值。

## 报告构建与打包

报告另需 XeLaTeX、TeX Live 的 ctex 等宏包、Latin Modern / TeX Gyre / Fandol 字体，以及 `requirements.txt` 中的 markdown-it-py。

```bash
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

实名和匿名 PDF 分别生成在 `build/texture-packing.pdf`、`build/texture-packing-anonymous.pdf`；LaTeX 和编译日志在 `build/md-report/`。构建器不修改 Markdown，也不覆盖提交 PDF。检查 PDF 内容、目录、分页及匿名信息后，才更新 `TA/yds_2026G17_P6.pdf` 与 `Peer Review/texture-packing.pdf`。

`make package-code` 先运行正确性检查，再仅同步 `code/` 中的两个源文件和提交目录 README，保留现有 PDF，并拒绝白名单之外的文件。报告中出现的 `scripts/` 路径指开发目录，不是提交包内容。

YiFeng Zheng 的审批流程及记录要求见 [opinion/README.md](opinion/README.md)。
