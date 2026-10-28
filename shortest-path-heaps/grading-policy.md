# Project 2: Grading Policy and Rubric

来源：用户于 2026-10-28 提供的课程评分标准。以下保留英文原文；适用于本目录的 Dijkstra + heaps 项目。Grading Policy 与六项 Rubric 各自合计 20 分，是同一总分的两种划分，不应相加。

本文件保存评分要求，不代表已按该细则重新审核当前代码、报告或提交包。历史模拟评分及其当时依据保留在 `opinion/`；后续审核应参考本文件中的完整细则。

## Grading Policy

**Programming:** Implement Dijkstra's algorithm with Fibonacci heap and other heaps (6 pts.). Write a test of performance program (3 pts.). **All the codes must be sufficiently commented.**

**Testing:** Provide the necessary inputs for testing and give the run time table (2 pts.). Plot the run times vs. input sizes for illustration (2 pts.). Write analysis and comments (3 pts.).

**Documentation:** Chapter 1 (1 pt.), Chapter 2 (2 pts.), and finally a complete report (1 point for overall style of documentation).

## 评分标准

Rubric items: 6

### Item 1: 1 point

The cover page must be presented with the title and the date of completion (+ 0.2 pts.). A complete problem description must be given in Chapter 1 (+0.8 pts.). Deduct points if:

- the cover page is not complete (-0.2)
- the introduction is a simple copy + paste of the assignment statement (-0.3)
- the introduction is not very clear -- in this chapter they are supposed to make it clear WHAT is to be done, besides why they are doing it (-0.1 ~ 0.5)
- others – please specify in the final comments.

### Item 2: 2 points

Chapter 2 is supposed to contain the descriptions of at least 2 heap structures (+1.4 pt.), together with all the key operations (+0.6 pt.). Deduct points if:

- the algorithm specification is not complete -- Fibonacci heap is not discussed; (-0.7)
- the algorithm specification is not complete – only one heap structure is discussed; (-0.7)
- the algorithm specification is not complete -- they are supposed to specify all the key operations of the chosen data structures; (-0.1 ~ 0.6)
- not making their algorithm easier to understand than a simple program (-0.5)
- only a program + comments, which is not acceptable (-1)
- others – please specify in the final comments.

### Item 3: 1 point

The overall style of documentation is supposed to be neat and clear. Deduct (at most 1) point if:

- the document looks like a patch work (-1)
- messy – some of the data in the charts and tables are missing (-1)
- the hand-in is not zipped with proper folders (-1)
- the hand-in is not complete – some files are missing (-1)
- others – please specify in the final comments.

### Item 4: 4 points

A complete table of testing data must be given in Chapter 3, including testing the correctness (+1 pts.) and the performance (+3 pts.). Deduct points if:

- the testing results are not complete – the correctness testing is missing(-1)
- the testing is run on a much simpler data set than the required benchmark set; (-1~3)
- others – please specify in the final comments.

### Item 5: 3 points

In Chapter 4, analysis of both the time (+1 pt.) and space (+1 pt.) complexities must be given, together with sufficient discussions on the testing results (+1 pt.). Deduct points if:

- the plot is missing; (-1)
- there is no comments on analyzing why one implementation is faster than another; (-1)
- **analysis** of the **complexities** of time and/or space is missing -- they must show how they have reached their conclusions, instead of simply listing them (-1)
- others – please specify in the final comments.

### Item 6: 9 points

The programmer is supposed to implement Dijkstra's algorithm with Fibonacci heap and other heaps (+6 pts.), together with a test of performance program (+3 pts.). Deduct points if:

- not all the necessary operations are implemented (-1 ~ 6)
- the testing program is missing (-3)
- some of the programs are not working properly (-1 ~ 3)
- the programs are not well-commented (-9)
- others – please specify in the final comments.
