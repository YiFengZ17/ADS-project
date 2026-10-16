# ADS 小组项目

YiFeng Zheng、YuJie Sun，Group 17。这里放 ADS 各次实验的代码、报告和提交版本。

## 怎么合作

每次实验一人写代码、运行实验并写报告，另一人拉取后复现、审阅，把结果和建议放进 `opinion/`。角色可以互换，两人都可以直接提交到 `main`。

开始前 `git pull --ff-only`，完成后检查 `git diff`，再 `git add <本次修改的文件>`、`git commit`、`git push`。尽量别同时改同一文件；推送冲突先整合更新，不强制推送。复现意见写明环境、命令和对应 commit，作者处理后在意见文件里回复。

如果自己已经提交、但 `git push` 被拒：工作区干净后执行 `git pull --rebase`；有冲突时保留双方有效修改，`git add <冲突文件>` 后 `git rebase --continue`，重新检查再 `git push`。开始前有未提交修改时先提交或 stash。

## 文件在哪

```text
ADS/
├── AGENTS.md / AGENT.md    # AI 入口与项目工作约定
├── templates/report/      # 打包好的课程报告排版模板
├── compare-trees/          # 第一次实验
└── shortest-path-heaps/    # 第二次实验：Dijkstra + 二叉堆 / Fibonacci 堆
    ├── code/              # 日常开发代码
    ├── report/report.md    # 报告正文
    ├── figures/           # 报告使用的图
    ├── TA/                # 实名提交版
    ├── Peer Review/       # 匿名提交版
    ├── opinion/           # 复现记录、意见和处理结果
    ├── scripts/           # 构建与分析入口
    ├── Makefile
    └── build/、data/       # 本地生成，不上传 Git
```

新实验沿用上面的子目录。模板用法见 `templates/report/README.md`；现有实验的依赖、运行和报告生成方法见 `compare-trees/README.md` 与 `shortest-path-heaps/README.md`。

本地输入、计时数据、构建缓存和 Claude 配置不上传。正式提交 PDF 和报告图保留在仓库，方便互相审阅。
