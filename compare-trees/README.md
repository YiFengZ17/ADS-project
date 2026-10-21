# Compare Search Trees

作者维护 `code/` 和 `report/report.md`；另一位成员复现并在 `opinion/` 留意见。两人都可以直接提交 main，开始前先拉取更新。

## 目录

- `code/`：五种树实现和 `test.c`，唯一开发源码；`code/examples/testavl.c` 保留原根目录的独立 AVL 练习，不参与实验构建或打包。
- `report/report.md`：唯一报告正文；`report/legacy-latex/`：旧 LaTeX 归档，不参与构建。
- `figures/`：当前报告的三张图，保留在 Git 中。
- `TA/`：实名提交快照，报告 `yds_2026G17_P1.pdf`。
- `Peer Review/`：匿名提交快照，报告 `compare-search-trees.pdf`。
- `opinion/`：复现记录和意见。
- `scripts/`：Markdown 构建、数据汇总和提交代码同步工具。
- `build/`、`data/`、`qa/`、`scratch/`：本地输出，不上传。

## 编译和复现

以下命令在本实验目录执行，支持 Linux / WSL。代码需要 GCC、make 和 POSIX 接口。

```bash
make                 # build/compare-trees；只编译 code/test.c
make check           # 五种树的 smoke test
make sanitize        # ASan + UBSan；build/compare-trees-sanitize
make benchmark       # data/results/raw.csv 和 data/inputs/；完整运行可能需几分钟
make analyze         # 校验 1050 行计时，生成 data/results/summary.csv
make analysis-check  # 检查统计脚本会拒绝缺失、重复和预热记录
make plots           # 校验数据并更新 figures/ 下的三组 PNG/PDF
```

每个场景、规模和树先完整预热 1 次，再正式测 10 次。CSV 只保留正式计时；输入目录包含 21 份预热输入（r-1）和 210 份正式输入。五种树共 1050 条正式计时。

`make analyze` 使用 Python 3 标准库，计算各阶段毫秒中位数和最小—最大范围，不更新报告或图片。`make plots` 需要 Matplotlib，只更新图片，不改 Markdown。新计时受设备和负载影响；raw.csv 不上传，复现者需重新运行 benchmark。

## 报告构建

需要 Python 3、XeLaTeX（TeX Live 的 ctex、常用宏包、Latin Modern 和 TeX Gyre 字体）。首次准备 Python 依赖：

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements.txt
```

修改 `report/report.md` 后执行：

```bash
make report PYTHON=.venv/bin/python
# 已有 markdown-it-py 的环境也可以直接 make report
```

`make draft` 是同一入口。生成 `build/md-report/report-from-md.tex`，XeLaTeX 编译三次，日志在 `build/md-report/compile.log`，最终报告是 `build/compare-search-trees.pdf`。脚本确认 Markdown 字节未变化；需要人工检查全文、表格、公式、目录和分页。正文普通单次换行转成分段，数学表达式可以用 `$...$`。

构建不会覆盖 TA 或 Peer Review 的现有 PDF。作者检查后再把实名 PDF 保存为 `TA/yds_2026G17_P1.pdf`；匿名报告必须另行去除封面、作者列表、签名等身份信息并检查，再保存到 `Peer Review/compare-search-trees.pdf`。本次提交 PDF 已同步精简后的算法流程与性能原因分析，计时表与图已更新为 2026-10-21 的十次重测结果。

## 更新提交代码

```bash
make package-code    # 先运行 check，再把 code/*.c 同步到 TA/code 和 Peer Review/code
```

同步代码后检查两个提交目录的 diff，运行 sanitizer，确认代码与报告一致，再提交。提交包自身的 README 提供独立 GCC 命令，不需要工作目录里的 Markdown 或 Python 工具。

完整工作约定见根目录 `AGENT.md`，排版模板在 `../templates/report/`。
