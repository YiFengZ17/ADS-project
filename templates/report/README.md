# ADS 报告模板包

这里集中保存课程 LaTeX 模板及参考 PDF，不是某次实验的正文。

- `main.tex`：封面及总入口；`main.pdf`：现有样式参考。
- `sections/`、`appendix/`：章节占位文件。
- `sections/references.tex`：按首次引用顺序填写的 IEEE 数字参考文献；`ads.bib` 保留为可选 BibTeX 数据示例（默认入口不调用）；`figures/`、`code/`：图片和代码占位目录。
- `latexmkrc`：XeLaTeX 编译配置。

在本目录运行 `latexmk -outdir=build main.tex` 可预览模板，输出在 `build/main.pdf`；`latexmk -outdir=build -c` 清理中间文件。需要 TeX Live、XeLaTeX、ctex、TeX Gyre 和 Latin Modern 字体。

新实验建立 code/、report/、TA/、Peer Review/、opinion/，把正文写在 report/report.md；参考这里的排版，复用 compare-trees/scripts/build_report.py 和 report-style.tex（脚本使用同样的实验目录约定）。不把模板文件混进 TA 或 Peer Review 提交包。

模板已统一章节编号，并把 References 放在 Author List 之前；实际交付以当次作业要求及根目录 AGENT.md 为准。main.pdf 为当前模板的样式参考；修改 main.tex 后重新编译、检查，再将 build/main.pdf 同步到 main.pdf。
