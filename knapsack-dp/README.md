# Project 4 — Improving Dynamic Programming for Knapsack

Group 17。编写：YuJie Sun（YujieSun）；审批：YiFeng Zheng（YifengZheng）。实现为 C++17：课上的 DP-1、教材 3.4 节 DP-with-Lists、3.5 节 DP-with-Upper-Bounds，以及 Bonus 的 $O(n+w^3)$ 小权重算法（Polak、Rohwedder、Węgrzycki, ICALP 2021）。只求最优值，不输出方案。

## 文件

- `code/knapsack.h`、`code/knapsack.cpp`：四种算法（另有 `lists-sorted`：按效率排序后的列表法，用于区分“排序”和“上界剪枝”的作用）。
- `code/main.cpp`：求解程序，`./knapsack NAME < instance` 或 `./knapsack all < instance`。
- `code/bench.cpp`：性能测试程序，对一个输入文件预热 1 次、正式运行 10 次，只计算法时间。
- `code/gen.cpp`：测试数据生成器（uncorrelated / weak / strong / inverse / subset 五类）。
- `scripts/check.py`：正确性测试（穷举对照、独立 Python DP 对照、生成数据交叉对照、非法输入）。
- `scripts/benchmark.py`：四组计时实验，数据写入 `data/*.json`，图写入 `figures/`。
- `figures/`：报告使用的计时图，提交 Git。
- `report/report.md`：报告正文的唯一权威来源。
- `scripts/build_report.py`、`scripts/report-style.tex`：沿用 Project 3 的构建器和仓库模板样式，支持单独生成匿名版。
- `TA/`：实名报告 `yds_2026G17_P4.pdf`、`code/` 和 `README.md`。
- `Peer Review/`：匿名报告 `knapsack-dp.pdf`、`code/` 和 `README.md`。
- `opinion/`：复现记录和审批意见。
- `build/`、`data/`：本地输出，不提交。

## 编译、验证和计时

以下命令从 `knapsack-dp/` 执行，需要 GCC 或 Clang（C++17，使用 `__int128`）、make 和 Python 3；测试脚本仅使用标准库，画图需要 matplotlib。

```bash
make
make check
make sanitize
make benchmark
python3 scripts/benchmark.py --plot-only
```

`make` 输出 `build/knapsack`、`build/bench` 和 `build/gen`。`check` 在终端报告 6 组共 3,312 个用例；`sanitize` 输出 `build/knapsack-san`（ASan/UBSan），并运行其中 586 个较小用例。

`benchmark` 是报告计时表和图的来源：四组实验（`size`、`range`、`small-n`、`small-w`），每组五类数据 × 3 个种子，每个输入预热 1 次、正式运行 10 次取中位数；某算法在某一规模的中位数超过 500 ms 后，不再测更大的规模。输入文件缓存在 `data/instances/`，结果写入 `data/<实验名>.json`，图写入 `figures/<实验名>.png/.pdf`，终端同时打印报告用的表格数据。全部实验约需 20 分钟。`--plot-only` 只从已有 JSON 重画图和表。新运行不会改写报告中的历史结果。

## 报告构建与打包

报告另需 XeLaTeX、TeX Live 的 ctex 等宏包、Latin Modern / TeX Gyre / Fandol 字体，以及 `requirements.txt` 中的 markdown-it-py。

```bash
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

实名和匿名 PDF 分别生成在 `build/knapsack-dp.pdf`、`build/knapsack-dp-anonymous.pdf`；LaTeX 和编译日志在 `build/md-report/`。构建器不修改 Markdown，也不覆盖提交 PDF。检查 PDF 内容、目录、分页及匿名信息后，才更新 `TA/yds_2026G17_P4.pdf` 与 `Peer Review/knapsack-dp.pdf`。

`make package-code` 先运行正确性检查，再仅同步 `code/` 中的五个源文件和提交目录 README，保留现有 PDF，并拒绝白名单之外的文件。报告中出现的 `scripts/` 路径指开发目录，不是提交包内容。

YiFeng Zheng 的审批流程及记录要求见 [opinion/README.md](opinion/README.md)。
