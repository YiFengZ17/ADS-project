# 复现与审批

- 编写负责人：YuJie Sun（YujieSun），负责代码、测试、报告及意见处理。
- 审批负责人：YiFeng Zheng（YifengZheng），负责独立复现和审阅。
- 当前状态：作者侧已完成代码、测试、实验和报告，并通过 `make check`、`make sanitize`；代码与实验由 Claude 按用户要求编写和运行。分工沿用"两人轮换"（Project 5 由 YiFeng Zheng 编写，本次由 YuJie Sun 编写），如需调整请在此说明。尚待 YiFeng Zheng 独立复现审批。

在本目录新建 `日期-YifengZheng.md`，记录：姓名、日期、`git rev-parse HEAD`、环境、执行命令、关键输出、代码与报告问题、审批结论。作者在同一记录的"作者处理"部分回复，注明修复 commit。

建议从 `texture-packing/` 执行 `make check`、`make sanitize`、`make benchmark`，另按 `TA/README.md` 在提交目录中编译运行。重点检查：

- 所有算法输出的摆放合法（不越界、不重叠），打印的高度与下界正确；`ffdh` 与 `ffdh-scan` 的摆放完全一致；
- 线段树 `LevelTree::first_fit` 是否总返回"最低的放得下的层"；
- Sleator 算法中宽度恰为 $W/2$、$W$ 为奇数时左右两半的处理；
- cut 类数据的最优高度为 $10n$，检查器据此核对 FFDH ≤ $1.7\cdot OPT+h_{\max}$、NFDH ≤ $2\cdot OPT+h_{\max}$、Sleator ≤ $2\cdot OPT+h_{\max}/2$；
- 报告第 2.5 节自行推导的 $H<2A/W+h_{\max}$ 是否对 NFDH、FFDH、BFDH 都成立；
- 报告中的计时与质量结论是否有实测数据支持。计时来自作者本机（Apple M5、macOS 26.6、Apple Clang 21.0.0），换机器后绝对时间会变，相对快慢和高度比值应一致（高度与机器无关）。

2026-11-25：作者侧完成与自查记录见 [2026-11-25-author-check.md](2026-11-25-author-check.md)。
