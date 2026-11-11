# Project 5 — Shopping With Coupons

Group 17。编写：YiFeng Zheng（YifengZheng）；审批：YuJie Sun（YujieSun）。实现为 C++17：贪心地从最便宜的"商品 + 优惠券"组合买起，用三种方法实现——暴力排序（对照基准）、最小堆（默认，提交用）和二分阈值法。题面整理和本项目对题意的理解见 [problem.md](problem.md)。

## 文件

- `code/main.cpp`：求解程序，包含三种方法和计时模式。`./coupons < input` 用堆法输出 `件数 余额`；`./coupons heap|search|brute < input` 指定方法；`./coupons METHOD RUNS < input` 为计时模式。
- `code/gen.cpp`：测试数据生成器（random / close / equal / far 四类），任何编译器生成同一份数据。
- `scripts/check.py`：正确性测试（手工用例、穷举所有子集对照、Python 排序对照、生成数据、非法输入、生成器可移植性、大规模输入）。
- `scripts/benchmark.py`：两组计时实验，数据写入 `data/*.json`，图写入 `figures/`。
- `tests/sample.in`：题面样例。
- `figures/`：报告使用的计时图，提交 Git。
- `report/report.md`：报告正文的唯一权威来源。
- `scripts/build_report.py`、`scripts/report-style.tex`：沿用 Project 4 的构建器和仓库模板样式，支持单独生成匿名版。
- `TA/`：实名报告 `yds_2026G17_P5.pdf`、`code/` 和 `README.md`。
- `Peer Review/`：匿名报告 `shopping-coupons.pdf`、`code/` 和 `README.md`。
- `opinion/`：复现记录和审批意见。
- `problem.md`：题面整理，只留在开发目录。
- `build/`、`data/`：本地输出，不提交。

## 编译、验证和计时

以下命令从 `shopping-coupons/` 执行，需要 GCC 或 Clang（C++17）、make 和 Python 3；测试脚本仅使用标准库，画图需要 matplotlib。

```bash
make
./build/coupons < tests/sample.in
make check
make sanitize
make benchmark
python3 scripts/benchmark.py --plot-only
```

`make` 输出 `build/coupons` 和 `build/gen`。样例应输出 `8 2`。`check` 在终端报告 7 组共 1,639 个用例；`sanitize` 输出 `build/coupons-san`（ASan/UBSan），并运行其中 354 个较小用例。

`benchmark` 是报告计时表和图的来源：`size`（固定 D = 10^6，N 从 250 到 10^5）和 `money`（固定 N = 10^5，D 从 10^3 到 10^7）两组实验，每组三类数据 × 3 个种子，每个输入预热 1 次、正式运行 10 次取中位数；暴力法只测到 N = 4000。输入文件缓存在 `data/instances/`，结果写入 `data/<实验名>.json`，图写入 `figures/<实验名>.png/.pdf`，终端同时打印报告用的表格数据。全部实验约需 2 分钟。`--plot-only` 只从已有 JSON 重画图和表。新运行不会改写报告中的数值。

## 报告构建与打包

报告另需 XeLaTeX、TeX Live 的 ctex 等宏包、Latin Modern / TeX Gyre / Fandol 字体，以及 `requirements.txt` 中的 markdown-it-py。

```bash
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

实名和匿名 PDF 分别生成在 `build/shopping-coupons.pdf`、`build/shopping-coupons-anonymous.pdf`；LaTeX 和编译日志在 `build/md-report/`。构建器不修改 Markdown，也不覆盖提交 PDF。检查 PDF 内容、目录、分页及匿名信息后，才更新 `TA/yds_2026G17_P5.pdf` 与 `Peer Review/shopping-coupons.pdf`。

`make package-code` 先运行正确性检查，再仅同步 `code/` 中的两个源文件和提交目录 README，保留现有 PDF，并拒绝白名单之外的文件。报告中出现的 `scripts/` 路径指开发目录，不是提交包内容。

YuJie Sun 的审批流程及记录要求见 [opinion/README.md](opinion/README.md)。
