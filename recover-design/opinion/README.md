# 复现与审批

- 编写负责人：YiFeng Zheng（YifengZheng），负责代码、测试、报告及意见处理。
- 审批负责人：YuJie Sun（YujieSun），负责独立复现和审阅。
- 当前状态：待 YuJie Sun 复现审批。本机自动检查不代表审批通过。

在本目录新建 `日期-YujieSun.md`，记录：姓名、日期、`git rev-parse HEAD`、环境、执行命令、关键输出、代码与报告问题、审批结论。作者在同一记录的“作者处理”部分回复，注明修复 commit。

建议从 `recover-design/` 执行 `make check`、`make sanitize`、`make benchmark`，另编译 TA/code/main.cpp。重点检查无连接器交叉、度数守恒、搜索回滚、复杂度论证以及报告的实测依据。审批前补充 PTA 判题结果；目前没有宣称隐藏测试通过。
