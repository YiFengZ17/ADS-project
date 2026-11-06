# 作者侧完成记录与自查

日期：2026-11-06。执行者：Claude（按用户 YuJie Sun 要求编写代码、运行实验并撰写报告）。本记录是作者侧自查，不代替 YiFeng Zheng 的独立复现审批，也不是官方成绩。

环境：macOS 26.6，Apple M5（10 核），Apple Clang 21.0.0（`g++` 指向 Clang），Python 3.9.6（markdown-it-py 3.0.0、matplotlib 3.9.4），TeX Live 2026（XeLaTeX）。

## 依据

- 题面：8-1 Improving Dynamic Programming for Knapsack（20 分细则 + Bonus）。
- 教材节选 Kellerer、Pferschy、Pisinger《Knapsack Problems》第 50–54 页（3.4 DP-with-Lists、3.5 DP-with-Upper-Bounds）。节选不含 Fig. 3.8，因此上界版本的细节（按效率排序、前缀和 + 二分求 LP 界、贪心初始下界）是本项目自己的实现选择，报告中已说明。
- Bonus：Polak、Rohwedder、Węgrzycki, ICALP 2021（阅读的是 arXiv 2105.04035v1；报告引用 LIPIcs 正式版，DOI 已通过 DataCite 核对）。报告只按内容引用邻近性结论，未写引理编号，避免与正式版编号不一致。
- 其余文献的书目信息通过 Crossref 核对。

## 执行的命令与结果

从 `knapsack-dp/` 执行：

```bash
make check        # PASS：6 组 3,312 个用例
make sanitize     # PASS：ASan/UBSan 下 586 个用例，无报错
make benchmark    # 四组实验约 1 小时；所有输入上各算法结果一致
make report
python3 scripts/build_report.py --compile --anonymous
make package-code
```

另按 `TA/README.md` 在提交目录中逐条执行编译、样例、sanitizer、生成器和 bench 命令，输出正常。实名 PDF 18 页，匿名 PDF 17 页，均逐页渲染检查了目录、表格、图和分页；匿名 PDF 与 `Peer Review/README.md` 不含姓名、组号。

## 需要审阅者留意

- 计时跳过规则：某算法在某规模三个种子的中位数超过 500 ms 后，不再测更大规模（报告中标“–”）。DP-1 另拒绝 $c>2\times10^8$ 的输入。
- 上界剪枝在 subset、strong、inverse 上随种子变化极大（例如 inverse $n=10^6$ 三个输入分别为 19 s、41 ms、41 ms），报告的中位数需结合误差棒阅读。
- Bonus 算法的贪心前缀用随机枢轴 quickselect，期望 $O(n)$；论文用确定性线性中位数。邻近性范围取 $2w^2$。
- 实验结束后只改动了 `best_exact` 的参数为 const 引用（不影响行为与性能），之后重新运行了 `make check` 和 `make sanitize`。

## 仍未完成

- YiFeng Zheng 的独立复现审批。
- 报告封面作者/审批人分工（YuJie Sun 编写、YiFeng Zheng 审批）是按本仓库“角色可互换”的约定推定的，如不符请告知后修改。
