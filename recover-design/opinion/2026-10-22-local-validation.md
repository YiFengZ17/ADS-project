# Project 3 本地检查记录

记录日期：2026-10-22。执行者：Codex，作者工作区自动检查；不代表 YuJie Sun 已复现或审批。

基线 commit：`cebcff51d6c6fcb9e241038daec000214c70da13`。本记录与 Project 3 初稿一起提交，具体版本以包含本文件的提交为准。

源码 SHA256：`bd32a5f59f936340b360535d832d558c33693aaff4f2905dc07c1d95de8d1531`。
报告 Markdown SHA256：`827b91d81f8aa97e5f41123aa920ac50a046e7290a3ac3f0220d41baeb3d4ca3`。

环境：WSL2 Linux 6.6.87.2；GCC 13.3.0；Python 3.13.13；AMD Ryzen 7 H 255。

## 已执行

在 `recover-design/` 下运行：

- `make check`：935 个求解用例与 3 个检查器反例全部通过。
- `make sanitize`：同套检查全部通过，无 ASan/UBSan 报错。
- `make benchmark`：5 类 50×50 输入各预热 1 次、正式 10 次；报告保留 2026-10-21 的实际结果。计时包含 Python 校验和进程开销。
- `make report` 与 `python3 scripts/build_report.py --compile --anonymous`：生成两份 8 页 A4 报告，Markdown 未变化。
- 渲染实名报告全部页面并检查匿名封面与末页；核对公式、表格、分页、19 个 PDF 书签及关键内容。未见溢出或缺字；TeX 日志中的 Fandol 字体脚本提示不影响本报告英文内容。
- 核对匿名 PDF 文本与元信息不含两位成员姓名或组号。
- 将已检查 PDF 同步到两份交付目录，并校验字节一致。
- 比对两份提交包的源码、检查器、题面和样例与开发文件完全一致。
- 独立编译 `TA/code/main.cpp` 并运行 `TA/scripts/check.py`：935 + 3 项检查通过。

## 待审批

YuJie Sun 尚未复现和签署审批。请另建实名复现记录，并记录审批时的 commit。

算法是完整回溯搜索，最坏时间指数级；当前结果不证明所有 50×50 输入的运行速度。尚未取得 PTA 隐藏判题结果，也未获知截图外的时间或内存限制。报告已列明这些限制。
