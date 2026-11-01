# Project 3 搜索性能修复与验证

日期：2026-11-01。处理者：Codex，按用户“你去解决”的指令修改。基线为 `5224b84db579b7e0aeafee8de23026e01dd42b52`；修复代码及报告与本记录同次提交。本记录不是 YuJie Sun 的审批，也不是 PTA 判题结果。

## 问题与修复

旧求解器在合法的 9×9 全 1 输入上超过 15 秒仍未返回；10×10 全 1、仅 (1,1)/(1,3) 改为 2 的输入约耗时 10.701 秒。这两例分别违反总度数偶性与二分图两侧度数和相等条件。旧传播只检查局部度数和交叉，无法提前排除这类全局矛盾。

新增候选图分量检查，但扩大测试后仍发现已知有解的密集 50×50 输入超时。因此最终实现保留几何建模，改用精确度数子集子句与交叉排斥子句，并在单个 C++17 文件中实现双监视文字传播、first-UIP 学习、非顺序回退、活动度选变量、相位保存和重启。已学习的冲突约束保留，避免反复探索同一失败组合；无第三方运行依赖、无求解时间截断、无超时即无解的逻辑。

候选图按真实边染色，未使用网格坐标奇偶代替二分性。奇环组件仍参与精确搜索。全部学习子句保留，最坏时间和内存仍可能指数增长；报告已更新这项限制，未沿用旧算法的线性总空间结论。

## 本机复现

环境：macOS 26.6 arm64、Apple Clang 21.0.0、Python 3.9.6。以下命令从 `recover-design/` 执行：

```bash
make check
make sanitize
python3 scripts/check_rollback.py
python3 scripts/check.py --binary build/recover-design --regressions
python3 scripts/check.py --binary build/recover-design --stress
python3 scripts/check.py --binary build/recover-design --benchmark-regressions
make package-code
```

Release 与 ASan/UBSan 的实际检查均通过：

```text
PASS: 968 solver cases; 3 validator rejection checks
PASS: 2000 Boolean formulas against exhaustive truth tables
```

968 个网格用例由旧有 935 例、9 个有解/无解回归、24 个固定种子构造有解压力用例组成。新增网格用例的单进程上限为 2 秒。2,000 个公式直接调用实际 BooleanSolver，比较穷举真值表并检查返回模型；包含无解、空子句、单位子句和多次决策的情况。macOS 对原 Linux sanitizer 命令的 `-no-pie` 给出忽略/弃用警告，无运行期 sanitizer 错误。

学习路径检查的真实输出为：

```text
Counts: conflicts, learned clauses, backtracks, undone assignments, multi-level jumps, restarts
rollback-a: [0, 0, 0, 0, 0, 0]
rollback-b: [0, 0, 0, 0, 0, 0]
learning-50: [412, 412, 414, 104093, 280, 2]
PASS: 3 search-path cases
```

旧回滚输入不再触发回滚，因此测试没有保留不适用的旧计数断言。learning-50 要求各项覆盖计数为正，避免因排序变化导致虚假的固定计数失败。报告记录的是此次观察值。

一次预热及十次测量的本机端到端中位数：9×9 奇总度数例为 1.663 ms，10×10 两色需求不等例为 1.528 ms；49×49 奇总度数、50×50 两色需求不等分别为 2.707 ms、2.796 ms。四个密集构造有解 50×50 输入在扩展 Release 检查中约为 39–58 ms。测量包含启动、序列化、输入输出和 Python 合法性检查，不能视作纯算法耗时或官方判题时间。

## 报告与交付

- 更新 Markdown 中的建模、搜索、正确性、复杂度、测试、计时归属与局限性；保留原 2026-10-21 计时表的全部数值，并明确其属于旧求解器。
- 标准 CDCL 机制引用 Eén/Sörensson 的原始论文，已核对作者论文与出版社书目信息。未将一般算法的正确性替代本实现测试。
- 两份报告均由当前 Markdown 生成，为 8 页；正文数学片段 41 个、文本片段 89 个已检查。已渲染并目视检查全部页面，未发现裁切、重叠或缺字；目录、书签和匿名身份信息单独核验。
- 报告构建增加同一字体的文件名回退，解决 macOS 下已安装 TeX 字体无法按族名定位的问题；没有替换字体家族。
- `make package-code` 同步三份源码和提交 README；TA/Peer Review 仅包含规定的 code/、README 和对应 PDF。
- 历史模拟评分不适用于本次新算法。当前报告不宣称已取得 PTA 接受或组员审批，真实状态仍记录在 opinion/README.md。

下一步外部验证为实际 PTA 判题及 YuJie Sun 独立复现。仓库未提供可用 PTA 题目链接和登录会话，本轮无法取得官方评测结果；无法代替组员签署真实审批。
