# Shortest Path Algorithm with Heaps

Author: YiFeng Zheng
YuJie Sun

Group number: 17

Date: 2026-10-16

## Abstract

This project compares two min-priority queues for point-to-point Dijkstra searches: a lazy binary heap and a Fibonacci heap with decrease-key. The implementations share one directed-graph representation and differ only in queue management. We evaluate three distance-weighted USA road networks from the 9th DIMACS Implementation Challenge. Each network receives 1,000 reproducible source-target pairs, and both algorithms answer exactly the same pairs. All 3,000 paired distances agree; boundary, randomized, and sanitizer checks also pass. On this Apple M4 system, mean binary-heap query time is 7.393-12.403 ms, whereas mean Fibonacci-heap time is 48.383-78.303 ms. The binary heap is 6.22-6.54 times faster by the ratio of mean times. We explain the algorithmic bounds, experimental procedure, measured variation, and limits of this conclusion. Official dataset download links are supplied without including the large data files.

# Introduction

## Background and Motivation

A weighted shortest-path query asks for a route whose arc weights have the minimum possible sum. Road-network queries are a useful setting because graphs are large, sparse, and naturally represented by outgoing arcs. Dijkstra's algorithm is applicable when all arc weights are nonnegative. It maintains tentative distances, repeatedly selects the smallest one, and makes that vertex's distance final [1]. The choice of min-priority queue changes the cost of these selections and of distance updates.

A binary heap is compact and simple, but its queue operations take logarithmic time. A Fibonacci heap has amortized constant-time insertion and decrease-key, with logarithmic amortized extract-min [2]. Those bounds suggest a theoretical benefit when a search performs many relaxations. Actual elapsed time can differ because queue operations have different constant costs and memory-access patterns. This experiment asks whether the Fibonacci heap's asymptotic advantage translates into lower query time on the selected road networks.

## Objectives and Scope

We implement two complete point-to-point Dijkstra solvers, test their answers, and compare their per-query running times. The binary solver uses the standard priority queue with duplicate entries; the Fibonacci solver holds one node per discovered vertex and uses decrease-key. Both use the same graph, query pairs, distance type, relaxation rule, and early-stop condition. We test the NY, BAY, and COL distance graphs with 1,000 pairs each, exceeding the assignment's minimum of 1,000 pairs. The central question is whether the Fibonacci heap's theoretical advantage improves elapsed time on these road-network queries.

The measured task is a single source-target shortest-path query, not all-pairs preprocessing or route reconstruction. Graph loading and pair generation are outside each timed interval. The implementation returns a distance, or `INF` if the target cannot be reached. This distinction matters when comparing results: an apparent speed advantage would be meaningless if one implementation answered a different problem.

# Data Structure / Algorithm Specification

## Directed Road Graph

The input is a directed weighted graph $G=(V,E)$ with $n=|V|$ vertices and $m=|E|$ arcs. Each arc has a nonnegative integer weight. We store outgoing arcs in adjacency lists. A point-to-point query asks for the minimum total weight from a source to a target, or `INF` when no directed path exists. Input vertex IDs are 1-based; internal array positions are 0-based.

## Dijkstra's Algorithm

Dijkstra's algorithm maintains a tentative distance for each vertex and a set of settled vertices [1]. Its required queue operations are insert, extract-min, and update after a shorter path is discovered. Starting from distance zero at the source, it extracts the smallest tentative distance and relaxes the outgoing arcs. The query ends when the target is extracted or the queue empties. Nonnegative weights make an extracted valid distance final [1].

## Binary Heap

A binary min-heap stores keys in an array and supports insert and extract-min. Our baseline uses C++ `priority_queue` of `(distance, vertex)` pairs. On an improved distance it inserts a new pair, leaving any older copy in the queue. When an old copy is later extracted, its key differs from the current distance and is skipped. This is a lazy alternative to an indexed decrease-key operation.

## Fibonacci Heap

A Fibonacci heap is a collection of heap-ordered trees with a pointer to the minimum root [2]. It supports insert, extract-min, and decrease-key. Our implementation uses circular sibling lists, consolidation of equal-degree roots, and cascading cuts after key decreases. Only discovered, unsettled vertices are active in the heap. For direct indexing, `fib_dijkstra` physically constructs a `Node` array for **all** $n$ vertices at the start of each query; unused nodes never become active heap members.

# Implementation and Algorithm Analysis

## Shared Search and Input Validation

Both solvers use the same parser, adjacency lists, 64-bit distance type, relaxation rule, and early stop at the target. The DIMACS parser requires exactly one valid `p sp n m` header before all `a u v w` records [3]. It checks each record's required fields, nonnegative header arc count, vertex range, nonnegative arc weight, and final arc count. Unknown records and malformed rows are rejected. The 64-bit distance array uses `INT64_MAX/4` as its infinity sentinel; the tested path lengths are far below that value.

A query starts with `dist[source]=0`. On a valid minimum extraction `(d,u)`, each outgoing arc `(u,v,w)` proposes `d+w`. If this is smaller than `dist[v]`, the solver changes the tentative distance and updates its queue. The binary queue may return stale entries, which are discarded. The Fibonacci queue holds one active node per discovered vertex and decreases its key instead of inserting a duplicate. Neither method uses a road-specific heuristic or preprocessing.

## Queue Operations and Complexity

The lazy binary queue may receive up to $O(m)$ entries from successful relaxations. Each insertion and removal costs $O(\log m)$; scanning arcs costs $O(m)$. A conservative per-query upper bound is $O((n+m)\log m)$ time and $O(n+m)$ graph-plus-query space [1]. The extra entries explain why the binary method uses more queue slots than an indexed version might, but they avoid maintaining an index for decrease-key.

The Fibonacci queue has amortized $O(1)$ insert and decrease-key and $O(\log n)$ extract-min [2]. At most $m$ arc relaxations and $n$ vertex extractions give $O(m+n\log n)$ amortized time. Graph storage, distances, states, and the preconstructed `Node` array together use $O(n+m)$ space. The number of **active** nodes can be smaller than $n$, but the physical node-array allocation is always $n$ per query. These bounds concern worst-case graph size and amortized operations; they do not predict the measured cost of a short target query.

## Heap Invariants and Correctness

Every finite tentative distance is the length of a discovered source-to-vertex path. Relaxation can only lower it. The binary queue retains at least one current entry for each discovered, unsettled vertex; obsolete entries are skipped. The Fibonacci queue stores its current key in the vertex's one active node. Each valid extract-min therefore selects the smallest tentative distance.

Suppose that a valid extracted vertex `u` had a shorter path than its extracted distance. On that path, take the first unsettled vertex. Its settled predecessor would have relaxed the connecting arc, giving it a tentative distance no larger than the shorter path length. With nonnegative weights, this value would be smaller than `u`'s extracted key, contradicting extract-min. Thus extracted distances are final [1]. This argument proves that stopping when the target is extracted gives the shortest distance, provided the queue operations maintain their invariants. The parser checks and randomized tests exercise the implementation; they are not a formal proof of every pointer operation.

## Code-Level Costs Relevant to Timing

`binary_dijkstra` creates and fills a distance array for each query. `fib_dijkstra` creates distance and state arrays **and initializes a `Node` array of size $n$**, even if the search reaches only a small part of the graph. The node pointers remain stable because this array is not resized during the query. In `FibHeap::consolidate`, each extract-min creates temporary `roots` and `by_degree` vectors; `FibHeap::pop` also creates a `children` vector when the removed root has children. Those allocations, the list traversal, and the node initialization fall inside the measured query interval. They are implementation facts, not measured percentages of the runtime gap.

# Testing Results

## Official Inputs and Query Set

The following files are the official distance-weighted USA road instances used here [3]. Only download links are listed; no graph file is uploaded with this report or its submission packages.

| Dataset | Direct download link |
| --- | --- |
| NY | [USA-road-d.NY.gr.gz](https://www.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.NY.gr.gz) |
| BAY | [USA-road-d.BAY.gr.gz](https://www.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.BAY.gr.gz) |
| COL | [USA-road-d.COL.gr.gz](https://www.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.COL.gr.gz) |

The header counts and average directed arcs per vertex are shown below. The three inputs grow in vertex and arc count, but they also represent different regions. They are not scaled copies of one graph.

| Graph | Vertices $n$ | Arcs $m$ | $m/n$ |
| --- | ---: | ---: | ---: |
| NY | 264,346 | 733,846 | 2.776 |
| BAY | 321,270 | 800,172 | 2.491 |
| COL | 435,666 | 1,057,066 | 2.426 |

For each graph, a `mt19937` generator seeded with `20261016` draws 1,000 source-target pairs uniformly from the valid vertex IDs. The generated vector is retained and passed unchanged to both algorithms. Sampling permits repeated vertices and pairs; no pairs were filtered based on distance. The CSV records each pair and its answer, making the exact benchmark workload reproducible with the same executable and seed. All sampled targets were reachable.

## Implementation and Timing Environment

| Item | Configuration |
| --- | --- |
| Experiment date | 2026-10-16 |
| CPU | Apple M4 |
| Operating system | macOS 26.3.1 |
| Compiler | Apple clang 21.0.0; C++17 |
| Optimized flags | `-O2 -Wall -Wextra -Wpedantic` |
| Timer | `std::chrono::steady_clock`, wall elapsed time |
| Pair generation | `mt19937`, seed `20261016` |
| Timed repetitions | 1 per pair and method; 1,000 pairs per graph |
| Execution order | All binary queries, then all Fibonacci queries, per graph |

A timer starts immediately before a Dijkstra call and stops immediately after it returns. The interval includes per-query array and node initialization, queue work, arc scans, and termination. It excludes decompression, parsing, pair generation, CSV writing, plotting, and report generation. This is an end-to-end query time, not the isolated cost of heap operations. Each pair is timed once; the 1,000 sampled pairs provide a workload distribution, not repeated timing uncertainty for one query.

The **TA package root** is the working directory for every command below. It contains `Makefile`, `code/main.cpp`, and `scripts/analyze.py` and `scripts/test_parser.py`. The anonymous package has the same executable layout. The commands build code, check correctness, download the three inputs, generate per-query CSVs, and create the summary table and figure. The historical CSVs are not included; running the commands creates new local results on the reader's machine.

```sh
make
make self-test parser-test sanitizer
mkdir -p data
for x in NY BAY COL; do
  curl -L --fail "https://www.diag.uniroma1.it/challenge9/data/USA-road-d/USA-road-d.$x.gr.gz" -o "data/USA-road-d.$x.gr.gz"
  gzip -cd "data/USA-road-d.$x.gr.gz" > "data/USA-road-d.$x.gr"
  ./build/benchmark --validate "data/USA-road-d.$x.gr"
  ./build/benchmark "data/USA-road-d.$x.gr" 1000 20261016 > "data/results-$x.csv"
done
python3 scripts/analyze.py
```

The three benchmark CSVs appear in `data/`. The analysis script checks paired distances and writes `data/summary.csv`, `data/distribution.csv`, and the PDF and PNG plot in `figures/`. Executables appear in `build/`. These generated files are local outputs, not submission contents.

Report compilation is a separate author workflow using the Markdown source and XeLaTeX in the development directory. It is not needed to reproduce the submitted program results.

## Correctness Checks

The hand-built graph exercises a seven-weight shortest path that improves an earlier ten-weight candidate, a zero-weight final arc, source equal to target, and an unreachable target. A separate path with two arcs of weight 2,000,000,000 checks that the returned distance is 4,000,000,000 rather than a 32-bit overflow. For randomized checking, 100 directed graphs with 30 vertices and 200 randomly weighted arcs each contribute 20 random source-target pairs. Both algorithms returned equal answers for all 2,000 small-graph pairs. AddressSanitizer and UndefinedBehaviorSanitizer reported no error during this self-test.

After the original timing run, a parser regression test was added on 2026-10-17. It checks rejection of a missing or repeated header, a truncated record, an arc before the header, and invalid header fields. The fixed parser also accepted all three original DIMACS files, with the vertex and arc counts shown above. These new validation checks do not change the historical timing values in the tables.

During official benchmarking, the program computes all binary answers, then checks each Fibonacci answer against the corresponding binary result. It exits with an error on the first mismatch. The three benchmark programs completed successfully, so all 3,000 paired distances matched. This equality is an empirical check, not an independent oracle: a shared parser or relaxation bug could affect both methods. The small hand-built cases address some such shared errors, while a formal proof and broader tests would be needed for stronger assurance.

## Runtime Table and Figure

The mean and median below are calculated from the 1,000 per-query intervals for each graph and method. Total time is the sum of those intervals, not an independently timed batch. Values are rounded to three decimals after aggregation.

| Graph | Vertices | Arcs | Heap | Mean (ms) | Median (ms) | Total (s) |
| --- | ---: | ---: | --- | ---: | ---: | ---: |
| NY | 264,346 | 733,846 | Binary | 7.393 | 7.286 | 7.393 |
| NY | 264,346 | 733,846 | Fibonacci | 48.383 | 47.353 | 48.383 |
| BAY | 321,270 | 800,172 | Binary | 9.425 | 8.699 | 9.425 |
| BAY | 321,270 | 800,172 | Fibonacci | 58.620 | 57.833 | 58.620 |
| COL | 435,666 | 1,057,066 | Binary | 12.403 | 12.317 | 12.403 |
| COL | 435,666 | 1,057,066 | Fibonacci | 78.303 | 76.375 | 78.303 |

![Mean shortest-path query time versus graph size](../figures/runtime-vs-size.pdf)

The plot places the three graph vertex counts on the horizontal axis and mean milliseconds per query on the vertical axis. Both series rise from NY to COL, while the Fibonacci series stays well above the binary series. Each point summarizes 1,000 queries; the lines connect measured points and do not imply measurements at intermediate graph sizes. The two methods share one vertical scale so their absolute costs can be compared directly.

## Variation and Paired Comparison

Query times vary substantially because source-target pairs cause different search extents. To show variation without hiding it in a mean, the table gives the 25th and 95th percentiles of the 1,000 individual timings. Percentiles use linear interpolation between sorted observations.

| Graph | Heap | 25th percentile (ms) | 95th percentile (ms) |
| --- | --- | ---: | ---: |
| NY | Binary | 3.433 | 14.212 |
| NY | Fibonacci | 23.385 | 92.836 |
| BAY | Binary | 4.301 | 17.367 |
| BAY | Fibonacci | 28.636 | 109.921 |
| COL | Binary | 6.174 | 23.632 |
| COL | Fibonacci | 40.150 | 146.637 |

The ratio of Fibonacci mean to binary mean is 6.54 for NY, 6.22 for BAY, and 6.31 for COL. This ratio compares two methods on identical pairs for each graph, but it is a ratio of aggregate means, not a claim that every pair has that exact speedup. The binary method also has the lower median and 95th-percentile time in all three networks. No sampled target was unreachable, so these comparisons describe reachable random pairs.

## Analysis, Limitations, and Improvements

The three graphs have only 2.426-2.776 arcs per vertex. For a sparse graph family with $m=\Theta(n)$, both the binary bound $O((n+m)\log m)$ and the Fibonacci bound $O(m+n\log n)$ reduce to $O(n\log n)$ [1], [2]. A stronger bound for general graphs therefore does not by itself predict a win here. In this code, `fib_dijkstra` initializes $n$ `Node` objects plus distance and state arrays for every query. `FibHeap::consolidate` constructs temporary `roots` and `by_degree` vectors, and `FibHeap::pop` may construct a `children` vector. The binary version uses an array-backed queue and only a distance array. All of those operations lie inside the timed interval. They are plausible contributors to the measured gap, but no operation-level timing or hardware counters were collected, so their individual contributions remain unmeasured.

First, the runs use one machine and one timing pass per pair. Scheduling, frequency changes, cache state, and other processes can perturb wall-clock timing. Alternating heap order and repeating the full experiment on a quiet machine would give more stable estimates. Second, the three networks differ in geography and average degree as well as size; the rising lines cannot establish a scaling law from size alone. More official road graphs and controlled graph families would help separate those effects. Third, the comparison is between the particular lazy binary implementation and this particular Fibonacci implementation. An indexed binary heap, different memory layout, or optimized Fibonacci heap could change the gap. These limits qualify the claim; they do not change the observed timings and answer agreement on the tested inputs.

# Conclusion

Both Dijkstra implementations, including the decrease-key Fibonacci heap, agreed on all 3,000 road-network query distances. The binary heap was 6.22-6.54 times faster by the ratio of mean times, making it the practical choice for these inputs and this code. Fibonacci heaps retain the stronger amortized bound [2]. Repeated, order-balanced runs would strengthen the performance conclusion.

# References

[1] T. H. Cormen, C. E. Leiserson, R. L. Rivest, and C. Stein, *Introduction to Algorithms*, 4th ed. Cambridge, MA, USA: MIT Press, 2022, ch. 22. [Publisher](https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/).

[2] M. L. Fredman and R. E. Tarjan, “Fibonacci heaps and their uses in improved network optimization algorithms,” *Journal of the ACM*, vol. 34, no. 3, pp. 596-615, 1987, doi: [10.1145/28869.28874](https://doi.org/10.1145/28869.28874).

[3] 9th DIMACS Implementation Challenge, “Shortest Paths: Benchmark Instances and File Formats.” [Download page](https://www.diag.uniroma1.it/challenge9/download.shtml); [format specification](https://www.diag.uniroma1.it/challenge9/format.shtml).

# Author List

YiFeng Zheng and YuJie Sun, Group 17.

## Division of Work

YuJie Sun was responsible for the main program: the two Dijkstra implementations, heap operations, graph input and benchmark code. YuJie also analyzed the runtime results and the relationship between the measured behavior and the code's data structures.

YiFeng Zheng was responsible for testing and the report. This included correctness and input-validation checks, organizing the reproducible experiment procedure, preparing the result tables and figure, and writing and revising the report from the recorded outputs and group analysis.

## Collaboration Summary

The code, tests, data analysis, and report were combined into the same submission. The named and anonymous packages carry the same executable source and reproduction instructions. The runtime table reports the original recorded run; later parser checks are identified separately rather than presented as part of that earlier timing run.

# Declaration

This Project 2 submission is presented by Group 17. The report states the algorithms, dataset sources, measurement procedure, observed results, and known limitations. The named group members are listed below for electronic submission.

# Signatures

YiFeng Zheng (typed name)

YuJie Sun (typed name)

# Source Code Appendix

The complete commented C++17 source is supplied as `code/main.cpp` beside this report in each submission package. Its main entry points are `read_graph`, `binary_dijkstra`, `FibHeap`, `fib_dijkstra`, `self_test`, and `main`. The exact compile and run commands are in the package `README.md`; the graph files and raw timing CSVs are obtained or generated locally and are not bundled.
