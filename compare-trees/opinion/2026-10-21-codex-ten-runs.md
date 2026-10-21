# 十次计时更新记录

- 执行者：Codex，本地运行；不代表另一位组员已复现。
- 日期：2026-10-21。
- 基础 commit：`6a95c16`；本次只修改计时驱动，五种树的实现未改。
- 环境：AMD Ryzen 7 H 255；WSL2 Linux 6.6.87.2；GCC 13.3.0；`-std=c11 -O2 -Wall -Wextra -Wpedantic`。
- 方法：每个场景、规模和树完整预热一次，再正式测十次。预热不写入计时 CSV；随机种子随重复编号变化，同一轮五种树共享输入，树的测试顺序轮换。
- 命令（在 compare-trees/）：`make check sanitize`、`make benchmark`、`make analysis-check plots`、`make package-code`、`make report`。
- 结果：五种树的 smoke test 和 ASan/UBSan 检查通过；1050 条正式计时通过完整性、次数、种子和总时间检查。231 份输入的排列与场景规则通过检查。十轮中每种树在五个执行位置各出现两次。
- 统计：分别计算插入、删除及总时间的中位数和最小—最大范围。总时间中位数来自每轮总时间，不是两个阶段中位数相加。
- 观察：N=100000 时，Splay 在两个有序场景的总时间中位数为 4.662、4.919 ms。随机场景 BST 为 31.499 ms，BPlus32 为 32.181 ms，差约 2.2%；报告按性能接近描述，不强调稳定排名。
- 输出：更新报告表格、三组 PNG/PDF 图、提交代码与 README。旧本地数据保存在 `data/before-ten-runs/`；新原始数据、环境、代码哈希和检查日志在 `data/results/`，均不加入提交包。
