# Project 2 审查意见与处理记录

- 审阅者：WorkBuddy（AI 辅助审阅，不代表课程教师或组员本人）
- 审查日期：2026-10-22
- 审查对象：`shortest-path-heaps/` 的代码、报告正文、提交包结构
- 复现基础：仓库 `main` 分支 tarball（`YiFengZ17/ADS-project`），编译与测试在本地副本执行

## 审查结论

实现层面没有发现正确性缺陷。编译零警告，self-test、parser-test、ASan 与 UBSan 全部通过，两种队列在 2,000 组随机小图查询上答案完全一致。提交包的匿名性、PDF 目录与页码、报告数字与正文的对应关系均正确。

问题集中在两处：提交包结构与仓库自己的规则不一致，以及报告对性能差距的解释没有经过测量。

## 已处理：计时数据统一到当前机器

原先报告主表记录的是 2026-10-16 在 Apple M4 上的一次测量。作者后来更换了开发机器，报告的机器信息与现有硬件不再对应，且无法在原机器上复现。经作者确认，改为在当前机器上重新测量全部计时数据，报告主表、变化分析、归因表和环境表现在全部来自同一次会话、同一台机器、同一批图。

新环境：Apple M5，macOS 26.6，Apple clang 21.0.0，编译选项 `-O2 -Wall -Wextra -Wpedantic`，种子 `20261016`，每个查询 1,000 组、每组重复 3 次取中位数。

| Graph | 二叉堆均值 (ms) | Fibonacci 均值 (ms) | 比值 |
| --- | ---: | ---: | ---: |
| NY | 7.798 | 44.854 | 5.75 |
| BAY | 9.790 | 57.195 | 5.84 |
| COL | 13.592 | 76.750 | 5.65 |

原 M4 记录为 NY 7.393 / 48.383、BAY 9.425 / 58.620、COL 12.403 / 78.303 ms。两组数据的比值接近，绝对时间因机器不同而不同，属于预期现象。

新增 `--repeat N` 选项：同一查询重复计时 N 次并取中位数。测量期间这台机器有其他用户占用，swap 已用尽，个别查询会因换页而出现几十毫秒的离群值；取中位数可以让单次干扰不支配某个查询的计时。这同时回应了原报告 Limitations 中自己提出的"repeating the full experiment on a quiet machine would give more stable estimates"。

## 已处理：提交包结构（对应 AGENT.md 第 22 行）

`AGENT.md` 规定 `TA/` 与 `Peer Review/` 只允许包含 `code/`、`README.md` 和对应的一份报告 PDF。原提交包多出 `Makefile`、`scripts/analyze.py` 和 `scripts/test_parser.py`，而 P1 的提交包严格符合这条白名单。

处理：移出这三个文件，README 改为直接编译命令，报告第 3.2 节的复现命令同步更新。两个提交包现在各只有 `code/main.cpp`、`README.md` 和一份 PDF，与 P1 一致。

## 已处理：性能归因方向（报告第 3.7 节与结论章）

原报告在分析章列出三个可能的差距来源：每次查询构造 $n$ 个 `Node` 对象、`consolidate()` 中的临时 vector、以及图的稀疏性，并说明"没有收集操作级计时，因此各自贡献未测量"。结论章却直接用 6.22-6.54 倍这个数字支撑"二叉堆是本场景更优选择"。

新增了 `--attribute` 模式，把每次查询拆成"分配初始化"与"实际搜索"两段分别计时，并统计两种队列的操作次数，同时加入索引二叉堆作为第三个求解器。测量在报告主表使用的同三个官方 9th DIMACS 路网（NY、BAY、COL）上进行。

结果与原报告的猜测方向相反：

- 每次查询构造完整 `Node` 数组只占差距的 0.7% 到 0.8%，不是主因；
- `FibHeap::consolidate` 每次 extract-min 都重建整个根环，在 COL 图上 1,000 次查询触发约 2.18 亿次合并和 15 亿次 link，占 Fibonacci 总耗时的 52.5% 到 54.3%；
- 索引二叉堆（带真正的 decrease-key）比懒删除二叉堆慢 1.13 到 1.22 倍，但比该 Fibonacci 实现快 4.8 到 5.3 倍。因此"二叉堆更优"这一结论不依赖于基线偏弱，差距来自具体实现而非 Fibonacci 堆本身。

据此重写了分析章与结论章，并加了一节说明归因实验的运行环境。

## 数据来源说明

官方下载站 `www.diag.uniroma1.it` 在审查期间持续返回 502 Bad Gateway，NY、BAY 和 COL 无法从原站取得。所用的三个图取自 GitHub 上的 `lbwang95/QHL4CSP` 仓库，其中存放的是同一批官方原始文件：文件首行为 `c 9th DIMACS Implementation Challenge: Shortest Paths`，header 分别为 `p sp 264346 733846`、`p sp 321270 800172`、`p sp 435666 1057066`，与报告主表记录的规模逐项一致，实际 arc 数也与 header 相符。

主表与归因表的数据现在都来自同一次测量，不再与原 M4 记录并列。

## 顺带处理的小问题

- `--attribute` 模式原先把索引二叉堆的初始化时间记为 0，实际它同样分配距离数组和 settled 数组。已改为分别计时，输出中该值不再为 0。
- 删除了 `FibHeap::count` 字段。它在 `push` 和 `pop` 中被维护，但没有任何地方读取，`empty()` 用的是 `minimum==nullptr`。
- 为 `Node` 的自引用初始化、`consolidate` 的根环快照、`swap(x,y)` 的不变量、`cascading_cut` 的 mark 规则、`decrease` 的 minimum 重定位、`IndexedHeap` 的 `sift_up`/`sift_down` 与 `pos` 数组补了行内注释。
- 补注 `by_degree` 初始容量 64 的来源：Fibonacci 堆节点度数为 $O(\log_\phi n)$，对本实验的图规模足够，代码另有动态扩容兜底。

## 已处理：措辞

报告第 1.2 节原写"exceeding the assignment's minimum of 1,000 pairs"，实际恰好是 1,000 个，改为"meeting"。

## 建议作者在本机重跑

官方数据源在审查期间持续返回 502 Bad Gateway，NY、BAY 和 COL 无法从原站取得。归因实验所用的三个图取自 GitHub 上的 `lbwang95/QHL4CSP` 仓库，其中存放的是同一批官方原始文件：文件首行为 `c 9th DIMACS Implementation Challenge: Shortest Paths`，header 分别为 `p sp 264346 733846`、`p sp 321270 800172`、`p sp 435666 1057066`，与报告主表记录的规模逐项一致，实际 arc 数也与 header 相符。

用这些文件测得的绝对时间与报告主表的历史记录高度吻合，说明代码与原始实验一致：

作者在自己的机器上重跑一次可以补上这层说明，并确认编译器版本下的行为一致：

```sh
make && make self-test parser-test
mkdir -p data
for x in NY BAY COL; do
  curl -L --fail "https://www.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.$x.gr.gz" -o "data/USA-road-d.$x.gr.gz"
  gzip -cd "data/USA-road-d.$x.gr.gz" > "data/USA-road-d.$x.gr"
  ./build/benchmark "data/USA-road-d.$x.gr" 1000 20261016 --repeat 3 > "data/results-$x.csv"
  ./build/benchmark --attribute "data/USA-road-d.$x.gr" 1000 --repeat 3
done
```

若官方站仍然不可用，可用镜像 `https://raw.githubusercontent.com/lbwang95/QHL4CSP/master/data/<图名>/USA-road-d.<图名>.gr` 取得同样的官方文件（未压缩版，跳过 `gzip -cd`）。

重跑后只需核对归因表的五个数字是否落在报告给出的范围内：初始化占比 0.7% 到 0.8%、`consolidate` 占比 52% 到 55%、索引二叉堆与懒删除二叉堆之比 1.13 到 1.22、Fibonacci 与懒删除二叉堆之比 5.8 到 6.2、Fibonacci 与索引二叉堆之比 4.8 到 5.3。若有偏差，把表中的数字换成本机的实测值即可，其余文字不必改动。

## 未处理，建议后续考虑

- 报告表格中的 `Total (s)` 列与 `Mean (ms)` 列数值相同（1,000 次查询下两者恰好相等），信息冗余，可删或加一句说明。
- 计时只跑一轮，且执行顺序固定为先全部二叉堆再全部 Fibonacci 堆。审查中在真实数据上测过三种顺序，比值在 6.27 到 8.26 倍之间波动，方向性结论不受影响，但补一句量化说明会更好。

## 审查过程中的一个自我更正

最初以为 `scripts/report-style.tex` 的字体声明在本机无法解析，尝试加入绝对字体路径。实测发现原文件可以直接编译，问题出在补丁引用了并不存在的 `lmmono10-bold.otf` 和 `FandolFang-Bold.otf`。该文件已恢复为原始内容，逐字节比对确认无差异。

另一次更正：`--attribute` 模式最初把索引二叉堆的初始化时间记为 0，而该解法同样分配距离数组。已改为分别计时，报告中引用的比值也据此重算过一次。
