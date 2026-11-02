# 复现与审批

- 编写负责人：YiFeng Zheng（YifengZheng），负责代码、测试、报告及意见处理。
- 审批负责人：YuJie Sun（YujieSun），负责独立复现和审阅。
- 当前状态：待 YuJie Sun 复现审批。本机自动检查不代表审批通过。

在本目录新建 `日期-YujieSun.md`，记录：姓名、日期、`git rev-parse HEAD`、环境、执行命令、关键输出、代码与报告问题、审批结论。作者在同一记录的“作者处理”部分回复，注明修复 commit。

建议从 `recover-design/` 执行 `make check`、`make sanitize`、`make benchmark`，另编译 TA/code/main.cpp。重点检查无连接器交叉、度数守恒、搜索回滚、复杂度论证以及报告的实测依据。审批前补充 PTA 判题结果；目前没有宣称隐藏测试通过。

2026-10-22 的模拟审核与处理记录：

- [第一轮模拟 TA 审核（93/100）](2026-10-22-subagent-review-round1.md)
- [第二轮模拟 TA 复审（95/100）](2026-10-22-subagent-review-round2.md)
- [作者工作区处理记录](2026-10-22-author-response.md)

上述评分采用 subagent 自拟标准，不是官方成绩，也不能替代 YuJie Sun 的审批。

2026-11-01：搜索性能修复与新的本地验证已完成，详见 [修复记录](2026-11-01-performance-fix.md)。仍待实际 PTA 判题和 YuJie Sun 独立审批。

2026-11-02：用户确认自己为 YuJie Sun，并要求按本项目既有标准重新自评。Codex 完成 [当前版本自评（98/100）](2026-11-02-self-evaluation.md)：恢复最大规模困难搜索及跨层回退的 3 分，仍保留完整网格学习后判无解的覆盖缺口和真实确认各 1 分扣分。本记录不代替用户本人审批，也不是课程官方成绩。

同日后续：[补齐搜索后无解案例](2026-11-02-learning-unsat.md)，独立穷举与 Release、ASan/UBSan 检查通过。测试项恢复满分，原实名交付口径为 99/100；匿名版本不要求签名、声明或身份确认，该项不得作为匿名交付扣分，本轮匿名版内部自评为 100/100。
