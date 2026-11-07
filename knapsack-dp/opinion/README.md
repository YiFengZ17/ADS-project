# 复现与审批

- 编写负责人：YuJie Sun（YujieSun），负责代码、测试、报告及意见处理。
- 审批负责人：YiFeng Zheng（YifengZheng），负责独立复现和审阅。
- 当前状态：YiFeng Zheng 已于 2026-11-07 提交复现意见（8 条，见下），作者已逐条处理并重跑全部实验；分工（YuJie Sun 编写、YiFeng Zheng 审批）已由审批人确认。审批结论仍待 YiFeng Zheng 本人填写。

在本目录新建 `日期-YifengZheng.md`，记录：姓名、日期、`git rev-parse HEAD`、环境、执行命令、关键输出、代码与报告问题、审批结论。作者在同一记录的“作者处理”部分回复，注明修复 commit。

建议从 `knapsack-dp/` 执行 `make check`、`make sanitize`、`make benchmark`，另按 `TA/README.md` 在提交目录中编译运行。重点检查：

- 四种算法在同一输入上结果一致，与穷举和独立 Python DP 一致；
- DP-with-Upper-Bounds 的上界是否确实不小于剩余物品能带来的利润（剪枝不能删掉更优解）；
- Bonus 算法的邻近性范围 $2w^2$ 与论文 Lemma 2.1 的对应关系，以及 SMAWK 的使用条件（凹序列）；
- 报告中的计时结论是否有实测数据支持。计时来自作者本机（Apple M5、macOS、Apple Clang），换机器后绝对时间会变，相对快慢应一致。

2026-11-06：作者侧完成与自查记录见 [2026-11-06-author-check.md](2026-11-06-author-check.md)。

2026-11-07：YiFeng Zheng 的复现记录与审阅建议见 [2026-11-07-YifengZheng.md](2026-11-07-YifengZheng.md)（由 Claude 代为运行；审批结论待 YiFeng Zheng 本人填写）。
