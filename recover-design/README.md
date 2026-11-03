# Project 3 — Recover the Design

Group 17。编写：YiFeng Zheng（YifengZheng）；审批：YuJie Sun（YujieSun）。已修复本地复现的搜索性能问题并更新提交包；2026-11-03 按 20 分评分细则修订代码注释、测试和报告（见 `opinion/2026-11-03-rubric-revision.md`）。尚未取得 PTA 隐藏测试结果。

沿用 Project 1 的目录和交付方式，报告 Introduction 直接说明 PTA 题意。开发目录中的 [problem.md](problem.md) 仅留作题面记录，不进入 TA 或 Peer Review 提交包。实现为 C++17，使用候选图分量检查、布尔约束编码和冲突学习搜索。最坏时间为指数级，不能仅凭本地用例通过就保证最大规模隐藏用例的性能。

## 文件

- `code/main.cpp`：求解器，读取一组输入。
- `code/gen.cpp`：大规模输入生成器（planted / broken / random 三类），随提交包一起提交。
- `tests/sample1.in`：截图中的完整样例输入；`tests/rollback-a.in`、`rollback-b.in`：现有测试中的两个固定回滚案例。
- `scripts/check.py`：独立合法性检查、小规模穷举对照和计时复现。
- `scripts/check_boolean.py`：对实际布尔引擎进行 2,000 个公式的独立真值表对照。
- `scripts/check_rollback.py`：对源码的本地副本插桩，检查学习、跨层回退和重启；不改求解决策。
- `scripts/benchmark_sizes.py`：用 `code/gen.cpp` 生成 10～50 规模输入，检查输出并计时，生成 `figures/runtime-vs-size.png/.pdf`。
- `figures/`：报告使用的计时图，提交 Git。
- `report/report.md`：报告正文的唯一权威来源。
- `scripts/build_report.py`、`scripts/report-style.tex`：复用 Project 1 构建器与仓库模板样式，支持单独生成匿名版。
- `TA/`：实名报告 `yds_2026G17_P3.pdf`、`code/` 和 `README.md`。
- `Peer Review/`：匿名报告 `recover-design.pdf`、`code/` 和 `README.md`。
- `opinion/`：实际复现记录和审批意见。
- `build/`、`data/`、`qa/`：本地输出，不提交。

## 编译、验证和计时

以下命令从 `recover-design/` 执行，需要 Linux / WSL、GCC（C++17）、make 和 Python 3；测试脚本仅使用标准库。

```bash
make
./build/recover-design < tests/sample1.in
make check
python3 scripts/check_rollback.py
make sanitize
make benchmark
python3 scripts/check.py --binary build/recover-design --regressions
python3 scripts/check.py --binary build/recover-design --stress
python3 scripts/check.py --binary build/recover-design --benchmark-regressions
make benchmark-sizes
```

`make` 输出 `build/recover-design` 和 `build/gen`。`check` 在终端报告 1,170 个网格用例、3 个检查器反例和 2,000 个布尔公式的结果；其中包括 152 个细长网格（2×m、n×2）和 50 个多解网格。原有网格用例设 15 秒超时，9 个回归和 24 个已知有解压力用例设 2 秒超时，求解器自身没有人为截断或将超时误报为无解。`sanitize` 输出 `build/recover-design-san` 并运行同一套检查。

`benchmark-sizes` 是报告计时表和图的来源：4 类输入 × 5 个规模 × 5 个种子，每个输入预热 1 次、正式运行 10 次，数据写入 `data/size-timings.json`，图写入 `figures/`。`benchmark` 每类输入预热 1 次、正式运行 10 次，在终端及 `data/timings.json` 输出毫秒值。计时包含进程启动、输入输出和 Python 合法性检查，不是纯求解时间。新运行不会改写报告中的历史结果。`--benchmark-regressions` 对 9 个回归用例采用同样的预热/重复次数，写入 `data/regression-timings.json`；`--regressions` 和 `--stress` 分别在终端输出逐例检查及耗时。布尔检查的临时测试程序位于 `build/boolean-check/` 和 `build/boolean-check-san/`。

回滚复现脚本在 `build/rollback-check/` 生成插桩源码、可执行文件和 `results.txt`。两个旧回滚输入在新引擎下无需回滚；新增的固定种子 50×50 用例验证了冲突学习、跨层回退和重启，具体计数见当前报告与本轮处理记录。

## 报告构建与打包

报告另需 XeLaTeX、TeX Live 的 ctex 等宏包、Latin Modern / TeX Gyre / Fandol 字体，以及 `requirements.txt` 中的 markdown-it-py。已有依赖的环境可直接运行：

```bash
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

Python 依赖可在虚拟环境内用 `python -m pip install -r requirements.txt` 安装；按根目录网络约束选择可用源。`make report PYTHON=...` 可指定解释器。

实名和匿名 PDF 分别生成在 `build/recover-design.pdf`、`build/recover-design-anonymous.pdf`；对应 LaTeX 在 `build/md-report/report-from-md.tex`、`report-anonymous.tex`，编译日志在同目录 `compile.log` 和 `compile-anonymous.log`。构建器不修改 Markdown，也不覆盖提交 PDF。

检查 PDF 内容、目录、分页及匿名信息后，才更新 `TA/yds_2026G17_P3.pdf` 与 `Peer Review/recover-design.pdf`。现有两个 PDF 已完成文风精简、模拟 TA 复审和排版检查。提交报告不显示待确认占位文字；匿名版直接省略封面身份字段及作者分工等身份章节。真实审批状态保留在 `opinion/`，删除占位文字不代表审批已完成。

`make package-code` 先运行正确性检查，再仅同步 `code/main.cpp`、`code/gen.cpp` 和提交目录 README，保留现有 PDF。打包脚本拒绝白名单之外的文件。提交包可按 README 独立编译和运行小型检查，输出写入外部临时目录；完整测试、回滚检查和计时脚本仅在开发目录中运行。报告中出现的 `scripts/`、`tests/` 路径指开发目录，不是提交包内容。

YuJie Sun 的审批流程及记录要求见 [opinion/README.md](opinion/README.md)。

2026-10-22 的两轮模拟审核针对旧求解器，不作为当前版本的评分。2026-11-01 的修复、验证与报告同步记录见 [本轮处理记录](opinion/2026-11-01-performance-fix.md)。这不替代 YuJie Sun 的审批。
