# Project 3 — Recover the Design

Group 17。编写：YiFeng Zheng（YifengZheng）；审批：YuJie Sun（YujieSun）。当前是代码、测试和报告初稿，待独立复现审批；尚未取得 PTA 隐藏测试结果。

沿用 Project 1 的目录和交付方式，题面转录见 [problem.md](problem.md)。实现为 C++17，使用度数约束传播和完整回溯搜索。最坏时间为指数级，不能仅凭本地用例通过就保证最大规模隐藏用例的性能。

## 文件

- `code/main.cpp`：唯一开发源码，读取一组输入。
- `tests/sample1.in`：截图中的完整样例输入；`tests/rollback-a.in`、`rollback-b.in`：现有测试中的两个固定回滚案例。
- `scripts/check.py`：独立合法性检查、小规模穷举对照和计时复现。
- `scripts/check_rollback.py`：对源码的本地副本插桩，复现两个案例的失败分支及回滚计数；不改求解决策。
- `report/report.md`：报告正文的唯一权威来源。
- `scripts/build_report.py`、`scripts/report-style.tex`：复用 Project 1 构建器与仓库模板样式，支持单独生成匿名版。
- `TA/`：实名报告 `yds_2026G17_P3.pdf`、代码及复现工具快照。
- `Peer Review/`：匿名报告 `recover-design.pdf`、代码及复现工具快照。
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
```

`make` 输出 `build/recover-design`。`check` 在终端报告 935 个求解用例和 3 个检查器反例的结果；对单个求解进程设 15 秒测试超时，求解器自身没有人为截断或将超时误报为无解。`sanitize` 输出 `build/recover-design-san` 并运行同一套检查。

`benchmark` 每类输入预热 1 次、正式运行 10 次，在终端及 `data/timings.json` 输出毫秒值。计时包含进程启动、输入输出和 Python 合法性检查，不是纯求解时间。新运行不会改写报告中的历史结果。

回滚复现脚本在 `build/rollback-check/` 生成插桩源码、可执行文件和 `results.txt`；两例分别撤销 5 和 6 条边赋值。它们来自现有 935 例测试，不能视为深层失败搜索的覆盖。

## 报告构建与打包

报告另需 XeLaTeX、TeX Live 的 ctex 等宏包、Latin Modern / TeX Gyre / Fandol 字体，以及 `requirements.txt` 中的 markdown-it-py。已有依赖的环境可直接运行：

```bash
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

Python 依赖可在虚拟环境内用 `python -m pip install -r requirements.txt` 安装；按根目录网络约束选择可用源。`make report PYTHON=...` 可指定解释器。

实名和匿名 PDF 分别生成在 `build/recover-design.pdf`、`build/recover-design-anonymous.pdf`；对应 LaTeX 在 `build/md-report/report-from-md.tex`、`report-anonymous.tex`，编译日志在同目录 `compile.log` 和 `compile-anonymous.log`。构建器不修改 Markdown，也不覆盖提交 PDF。

检查 PDF 内容、目录、分页及匿名信息后，才更新 `TA/yds_2026G17_P3.pdf` 与 `Peer Review/recover-design.pdf`。现有两个 PDF 都是经过文风精简和模拟 TA 复审、已检查排版的初稿，报告中的声明、签名和审批仍待成员确认。

`make package-code` 先运行正确性检查，再同步源码、检查器、样例和提交目录 README。提交包可依照各自 README 独立编译和复现，不依赖开发目录的 Markdown 构建环境。

YuJie Sun 的审批流程及记录要求见 [opinion/README.md](opinion/README.md)。

本轮已参照 Project 1 精简英文，并完成两轮独立 subagent 模拟审核（93/100 → 95/100，自拟标准，非官方评分）。最终报告为7页；审核意见及处理记录见 [opinion/README.md](opinion/README.md)。这不替代 YuJie Sun 的审批。
