# 复现与审批

- 编写负责人：YiFeng Zheng（YifengZheng），负责代码、测试、报告及意见处理。
- 审批负责人：YuJie Sun（YujieSun），负责独立复现和审阅。
- 当前状态：作者侧已完成代码、测试、实验和报告，并通过 `make check`、`make sanitize`；代码与实验由 Claude 按用户 YiFeng Zheng 的要求编写和运行。尚待 YuJie Sun 独立复现审批；尚未取得在线评测结果。

在本目录新建 `日期-YujieSun.md`，记录：姓名、日期、`git rev-parse HEAD`、环境、执行命令、关键输出、代码与报告问题、审批结论。作者在同一记录的"作者处理"部分回复，注明修复 commit。

建议从 `shopping-coupons/` 执行 `make check`、`make sanitize`、`make benchmark`，另按 `TA/README.md` 在提交目录中编译运行。重点检查：

- 三种方法在同一输入上输出完全一致，并与穷举所有子集、Python 排序一致；
- 堆法"每行只放一个组合、第一列买掉后才开下一行"是否总能取到当前最便宜的组合；
- 二分阈值法在总花费超过 D 时提前停止，是否避免了溢出（N² 个组合的总花费可达 10^19）；
- 对题意的理解：本项目按样例的做法，每次购买都用一张优惠券（见 `problem.md`），报告中不单列为假设。提交在线评测后如发现需要"用完所有组合后按原价继续买"，需补一步；
- 报告中的计时结论是否有实测数据支持。计时来自一台 Linux 机器（Intel Xeon 2.10 GHz、GCC 13.3.0），换机器后绝对时间会变，相对快慢应一致。

2026-11-11：作者侧完成与自查记录见 [2026-11-11-author-check.md](2026-11-11-author-check.md)。
