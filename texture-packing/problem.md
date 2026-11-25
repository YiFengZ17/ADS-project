# Project 6 题面：Texture Packing（20 分）

> 题面整理，只留在开发目录，不进入提交包。

Texture Packing is to pack multiple rectangle shaped textures into one large texture. The resulting texture must have a given width and a minimum height.

This project requires you to design an approximation algorithm that runs in polynomial time. You must generate test cases of different sizes (from 10 to 10,000) with different distributions of widths and heights. A thorough analysis on all the factors that might affect the approximation ratio of your proposed algorithm is expected.

## Grading Policy

- Programming: Implement the approximation algorithm (6 pts.). Write a test of performance program (1 pts.). All the codes must be sufficiently commented.
- Testing: Generate the test cases and give the run time table (2 pts.). Plot the run times vs. input sizes for illustration (2 pts.). Write analysis and comments (5 pts.). The analysis must be based on your testing results.
- Documentation: Chapter 1 (1 pt.), Chapter 2 (2 pts.), and finally a complete report (1 point for overall style of documentation).
- Bonus: Compare your algorithm to any of the known strip algorithms. Anyone who does excellent job on answering the Bonus question will gain an extra point.

## 本项目对题意的理解

- 纹理不能旋转（题面未允许旋转，按经典 strip packing 处理），坐标与边长均为正整数。
- 输入：第一行 `n W`，之后 `n` 行 `w h`（$1\le w\le W$）。输出：总高度，以及每个纹理左下角坐标。
- 主算法：FFDH（First-Fit Decreasing-Height），用线段树找第一层可放位置，$O(n\log n)$。
- Bonus 对比：NFDH、BFDH、Sleator 2.5 近似算法；另保留朴素扫描版 FFDH 作为计时对照。
