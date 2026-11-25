# Texture Packing

Author: YuJie Sun
Reviewer: YiFeng Zheng

Group number: 17

Date: 2026-11-25

## Abstract

We pack $n$ rectangular textures into a strip of fixed width $W$ and try to keep the height small. Our algorithm is First-Fit Decreasing-Height (FFDH): sort the textures from tall to short and put each one on the lowest "level" where it still fits. With a small tree over the levels it runs in $O(n\log n)$ time, and its height is at most $1.7\cdot OPT+h_{\max}$. We tested it on four kinds of generated data with $n$ from 10 to $10^5$ and compared it with NFDH, BFDH and Sleator's algorithm. For $n\geq1000$ FFDH is within 8% of a lower bound on every test input, and on inputs whose best height we know it is within 1.2% at $n=10^4$.

# Introduction

A game or a graphics program often has many small pictures, called textures. Loading them one by one is slow, so they are put together into one large texture. The large texture has a fixed width, and we want its height to be as small as possible so that it uses little memory.

So the problem is this. We get a width $W$ and $n$ rectangles with widths $w_i$ and heights $h_i$, where $1\leq w_i\leq W$. We must give each rectangle a position $(x_i, y_i)$ for its lower-left corner, such that every rectangle lies inside the strip ($0\leq x_i$, $x_i+w_i\leq W$, $y_i\geq0$) and no two rectangles overlap. We do not rotate the textures. The height of the packing is the highest top edge, $\max_i(y_i+h_i)$, and we want to make it small. This is the classic strip packing problem.

Our program reads `n W` on the first line and then one line `w h` for every texture. It prints the height and a simple lower bound on the first line, and then the corner `x y` of every texture in input order.

The hard part is that strip packing is NP-hard: even when all heights are equal it contains the bin packing problem, so we can not hope for an exact algorithm that is fast on 10,000 textures. Trying all orders is hopeless too, since there are already $10!\approx3.6\times10^6$ orders of only 10 textures. So we use an approximation algorithm: a fast rule whose height is never too far from the best height $OPT$, and we measure how far it is in practice.

# Algorithm Specification

## Level Packing

All the algorithms in this project except one are *level* algorithms. A level is a horizontal row: its bottom is a straight line, the textures stand on this line next to each other from left to right, and the level is as tall as its first (tallest) texture. The next level starts right on top of it.

We first sort the textures by height from tall to short (equal heights go wider first). Then the first texture of every level is the tallest one on it, so the textures on one level never stick out into the next level. The algorithms only differ in which level a new texture goes to.

## FFDH, Our Algorithm

**Why.** The simplest rule (NFDH, see 2.3) only looks at the newest level. When a texture does not fit there, the space left on all older levels is lost for ever. FFDH [1] fixes this: a short texture that comes later can still go back down to an old level that has a gap at its right end.

**Rule.** Take the textures from tall to short. Put each one on the lowest level that still has enough free width. If no level has room, open a new level on top and put the texture there; the new level is as tall as this texture.

**Finding the lowest level fast.** If we check the levels one by one, one texture can cost as many steps as there are levels, and that may be close to $n$. So we keep the free width of all levels in a tree:

1. **Tree.** The leaves are the levels in order from bottom to top. Every inner node stores the largest free width among the leaves below it. Levels that are not opened yet store $-1$.
2. **Search.** To place a texture of width $w$, look at the root. If its value is smaller than $w$, no level fits and we open a new one. Otherwise go down: go to the left child if its value is at least $w$, else to the right child. The leaf we reach is the lowest level that fits.
3. **Update.** After placing the texture, the free width of this level gets smaller. We write the new value into the leaf and recompute the nodes on its path to the root.

For example, let $W=10$ and the levels have free widths $3, 0, 6, 5$. The root stores 6, the left child $\max(3,0)=3$ and the right child $\max(6,5)=6$. For a texture with $w=4$ the left child is too small, so we go right and then left, and reach level 3. This is exactly the first level from the bottom with free width at least 4.

Both the search and the update walk one path of the tree, so each takes $O(\log n)$ steps. We also kept the plain version that scans the levels one by one (`ffdh-scan`). It gives exactly the same packing and we use it to measure what the tree saves.

## Other Strip Algorithms for Comparison

For the bonus part we also wrote three known algorithms.

**NFDH** (Next-Fit Decreasing-Height) [1] uses the same order but only looks at the newest level. If the texture does not fit there, a new level is opened, and older levels are never used again.

**BFDH** (Best-Fit Decreasing-Height) is the best-fit form of the same idea. Among all levels that have room, it picks the one with the least free width, so that big gaps are kept for wide textures. We keep the levels in a balanced search tree (`std::set`) ordered by free width, so that the best level is found in $O(\log n)$ time.

**Sleator's algorithm** [2] is not a pure level algorithm.

1. **Wide textures.** Every texture wider than $W/2$ is stacked at the bottom, one on top of another, since no two of them can stand side by side anyway.
2. **One full level.** On top of the stack, the other textures (from tall to short) form one level over the whole width, until the next one does not fit.
3. **Two halves.** The rest of the strip is split into a left and a right half. Each time we fill one level of width $W/2$, always in the half whose top is lower now, and continue until all textures are placed.

Sleator proved that its height is at most $2\cdot OPT+h_{\max}/2\leq2.5\cdot OPT$ [2], which is a better guarantee than FFDH has when $n$ is small.

## Lower Bound for Measuring Quality

We do not know $OPT$ for most inputs, so we compare with a lower bound $LB$, the largest of three simple values:

- the total area divided by $W$, rounded up, because the strip must hold all the area;
- the tallest texture $h_{\max}$;
- the total height of the textures wider than $W/2$, because no two of them fit side by side.

Since $LB\leq OPT$, the value $H/LB$ is never smaller than the real ratio $H/OPT$, so it is a safe way to report quality. For one data type (`cut`, see 3.3) we also know $OPT$ exactly.

## Correctness and Approximation Ratio

**Legal packing.** In every level algorithm a texture is placed only when the used width plus its own width is at most $W$, so it stays inside the strip and does not overlap its left neighbour. A level is as tall as its first texture, and the later textures are not taller because of the sorting, so no texture reaches into the level above. The same holds in each half of Sleator's algorithm. The wide textures in step 1 are simply stacked.

**Tree search.** The search only moves right when the whole left subtree has no level with free width $w$ or more, so the leaf it reaches is the leftmost (lowest) level that fits. That is why `ffdh` and `ffdh-scan` give the same packing, which we also check in the tests.

**A bound we derive ourselves.** Let the levels have heights $h_1\geq h_2\geq\dots\geq h_k$, and let $A$ be the total area. When level $i+1$ was opened, its first texture $t$ did not fit on level $i$. So the textures on level $i$ at that moment, together with $t$, are wider than $W$. All of them are at least $h_{i+1}$ tall, so their total area is more than $W\cdot h_{i+1}$. If we add this up for $i=1,\dots,k-1$, each texture is counted at most twice (once on its own level, once as the first texture of the next level), so

$$W\,(h_2+\dots+h_k) < 2A \quad\Rightarrow\quad H = h_1+(h_2+\dots+h_k) < h_{\max}+\frac{2A}{W}\leq h_{\max}+2\cdot OPT.$$

This argument holds for NFDH, FFDH and BFDH, because in all three a new level is only opened when the texture fits on no earlier level that is still in use. It gives the known NFDH bound [1].

**The FFDH bound.** With a finer counting argument, Coffman et al. proved that $H_{\mathrm{FFDH}}\leq1.7\cdot OPT+h_{\max}$ [1]. Since $h_{\max}\leq OPT$, this also means $H_{\mathrm{FFDH}}\leq2.7\cdot OPT$ for every input. When $n$ is large and each texture is small compared with the whole packing, the $h_{\max}$ term matters little and the ratio is at most about 1.7. We do not repeat the proof, but we check the bound on every input with known $OPT$ in our tests.

# Testing Results

## Method

We tested on macOS 26.6 with an Apple M5 CPU, Apple Clang 21.0.0 and Python 3.9. We compile with `-O2`, and also once with AddressSanitizer and UndefinedBehaviorSanitizer to find memory errors.

The checker is a separate Python script. For every output it checks that each texture is inside the strip, that no two textures overlap (with a sweep line over $x$), that the printed height is the real highest top edge, and that the printed lower bound is right. It also runs all algorithms again in Python, written in the simplest way (scan all levels), and the corners must be exactly the same. Finally it checks the bounds: every level algorithm must stay below $2A/W+h_{\max}$, and when $OPT$ is known, FFDH must stay below $1.7\cdot OPT+h_{\max}$, NFDH below $2\cdot OPT+h_{\max}$ and Sleator below $2\cdot OPT+h_{\max}/2$.

## Correctness Tests

| Test group (count) | Result | Purpose |
| --- | --- | --- |
| Hand-made cases (10) | PASS | Small cases with heights worked out by hand: one texture, all textures as wide as the strip, one full level, one texture too many, a case where first fit and best fit differ, width exactly $W/2$, an odd $W$. |
| Random small inputs (1,000) | PASS | $n\leq40$, $W$ from 1 to $10^9$, many equal sizes; same corners as the Python versions. |
| Generated inputs (160) | PASS | All four data types with $n$ from 1 to 2,000 and three width/height ranges; legal packings, same as Python, bounds hold. |
| Large inputs (8) | PASS | $n=10^4$ and $10^5$ of every type: legal packings and the bounds. The two FFDH versions must agree. |
| Bad inputs (9) | PASS | Empty input, $n=0$, missing numbers, $w>W$, zero sizes, extra numbers, an unknown algorithm name: all must give an error. |
| Generator check (16) | PASS | The generator must give the same data on every compiler. We compare it with a Python version. |

All 1,203 cases pass. With the sanitizers we run a smaller set of 217 cases and there is no error. Three of the hand-made cases ($W=10$, each texture written as width × height, expected heights):

| Input | FFDH | NFDH | BFDH | Sleator |
| --- | --- | --- | --- | --- |
| $4\times3$, $6\times2$, $5\times2$, $3\times1$, $10\times1$ | 6 | 7 | 6 | 6 |
| $6\times5$, $5\times3$, $4\times1$, $4\times1$ | 8 | 9 | 8 | 8 |
| $5\times3$, $6\times2$, $4\times2$, $5\times1$ | 6 | 6 | 5 | 5 |

The heights in the table are worked out by hand, and the program printed exactly these heights.
In the second case the last $4\times1$ texture goes back down to the first level in FFDH, while NFDH must open a third level. In the third case FFDH puts the $4\times2$ texture into the bigger gap, so the last $5\times1$ texture fits nowhere; BFDH puts it into the smaller gap and saves one level.

## Test Data

All large inputs come from our generator `code/gen.cpp`. The strip width is always $W=1000$. It takes $n$, a type, a seed and optionally the largest width $w_{\max}$ and height $h_{\max}$ (both 1000 by default). The four types are:

1. **uniform**: widths from 1 to $w_{\max}$ and heights from 1 to $h_{\max}$, both uniform.
2. **equalh**: uniform widths, but every height is $h_{\max}$. Level algorithms then waste no space in height, only at the right end of each level.
3. **heavy**: uniform widths, and height $h_{\max}/k$ with $k$ from 1 to 100, so most textures are short and a few are very tall.
4. **cut**: we take a $1000\times10n$ rectangle and cut it into $n$ pieces (always cutting the largest piece across its longer side at a random place), then shuffle the pieces. The pieces fill the rectangle exactly, so $OPT=10n$ and we know the real ratio.

## Run Time

The program runs one algorithm once to warm up and then 10 more times, and takes the median. A run that is shorter than 1 ms is repeated until it takes 1 ms, and we divide by the number of repeats. We time only the algorithm with its sorting, not reading the input. For every setting we use three seeds and report the median of the three. The plain-scan FFDH is only run up to $n=3\times10^4$.

**Table 1.** Median time in µs on uniform data.

| Algorithm | $n=10$ | $n=100$ | $n=1000$ | $n=10^4$ | $n=10^5$ |
| --- | --- | --- | --- | --- | --- |
| FFDH (ours, tree) | 0.33 | 3.31 | 41.2 | 651 | 10,400 |
| FFDH, plain scan | 0.24 | 2.35 | 80.4 | 5,930 | – |
| NFDH | 0.063 | 1.36 | 12.9 | 241 | 6,140 |
| BFDH | 0.44 | 4.33 | 51.2 | 1,140 | 16,500 |
| Sleator | 0.15 | 1.58 | 13.3 | 276 | 6,160 |

**Table 2.** Median time of FFDH in µs on each data type.

| Type | $n=10$ | $n=100$ | $n=1000$ | $n=10^4$ | $n=10^5$ |
| --- | --- | --- | --- | --- | --- |
| uniform | 0.33 | 3.31 | 41.2 | 651 | 10,400 |
| equalh | 0.32 | 3.37 | 37.5 | 570 | 8,400 |
| heavy | 0.35 | 3.39 | 39.4 | 744 | 10,300 |
| cut | 0.27 | 4.38 | 53.1 | 856 | 12,700 |

Figure 1 shows the times of all algorithms for 12 sizes from 10 to $10^5$. Both axes are on a log scale, so a straight line of slope 1 means linear growth and slope 2 means quadratic growth. The error bars show the fastest and the slowest of the three seeds.

![Run time against the number of textures n (log-log), one panel per data type](../figures/time.pdf)

## Packing Quality

**Table 3.** Height divided by the lower bound, mean of three seeds. For the cut type this is the real ratio $H/OPT$.

| Type | Algorithm | $n=10$ | $n=100$ | $n=1000$ | $n=10^4$ |
| --- | --- | --- | --- | --- | --- |
| uniform | FFDH | 1.270 | 1.107 | 1.056 | 1.026 |
| uniform | NFDH | 1.368 | 1.313 | 1.327 | 1.300 |
| uniform | BFDH | 1.270 | 1.093 | 1.041 | 1.016 |
| uniform | Sleator | 1.355 | 1.324 | 1.331 | 1.324 |
| equalh | FFDH | 1.272 | 1.034 | 1.010 | 1.003 |
| equalh | NFDH | 1.272 | 1.321 | 1.291 | 1.287 |
| equalh | Sleator | 1.414 | 1.363 | 1.326 | 1.321 |
| heavy | FFDH | 1.357 | 1.129 | 1.070 | 1.023 |
| heavy | NFDH | 1.433 | 1.226 | 1.285 | 1.282 |
| heavy | Sleator | 1.381 | 1.271 | 1.314 | 1.310 |
| cut | FFDH | 1.000 | 1.210 | 1.067 | 1.011 |
| cut | NFDH | 1.000 | 1.261 | 1.127 | 1.073 |
| cut | Sleator | 1.000 | 1.220 | 1.170 | 1.154 |

On equalh, heavy and cut data BFDH is within 0.005 of FFDH in these columns, so we leave it out of the table. Figure 2 shows the same ratio for all 12 sizes, with the smallest and largest of the three seeds as error bars.

![Height / lower bound against n, one panel per data type](../figures/ratio.pdf)

To see the effect of the texture width, we fix $n=1000$ and uniform heights, and change $w_{\max}$ from 50 to 1000 (Figure 3, 10 seeds each).

![Height / lower bound against the largest texture width, n = 1000](../figures/width.pdf)

Figure 4 shows what the packings look like for one small uniform input ($n=40$, $w_{\max}=400$, $h_{\max}=300$; the lower bound is 1423). The dashed line is the height.

![Packings of the same 40 textures by NFDH, FFDH and Sleator's algorithm](../figures/example.pdf)

# Analysis and Comments

## Time Complexity

Sorting the textures takes $O(n\log n)$ [3, ch. 2]. In FFDH there are at most $n$ levels, so the tree has height $O(\log n)$, and each texture does one search and one update, $O(\log n)$ each. So FFDH takes $O(n\log n)$ in total.

The plain-scan FFDH checks up to $L$ levels for each texture, where $L$ is the number of levels, so it takes $O(n\log n+nL)$. In the worst case, for example when every texture is wider than $W/2$, $L=n$ and this is $O(n^2)$. NFDH only looks at the newest level, so after sorting it is $O(n)$, and the total is $O(n\log n)$. BFDH does one search, one removal and one insertion in a balanced search tree per texture, $O(\log n)$ each [3, ch. 13], so it is also $O(n\log n)$. Sleator's algorithm sorts once and then places each texture in $O(1)$ time, so it is $O(n\log n)$ as well [2].

The measured times agree with this. From $n=10^3$ to $10^4$ FFDH becomes 15 to 19 times slower, and from $10^4$ to $10^5$ 14 to 16 times slower, depending on the data type. $n\log n$ grows by about 13 times for each step; the small extra is probably because larger inputs no longer fit in the fast cache. In contrast, the plain scan on uniform data goes from 80.4 µs to 5,930 µs from $n=10^3$ to $10^4$ (74 times) and to 51,700 µs at $3\times10^4$ (8.7 times for 3 times more textures), which is the quadratic growth we expect when $L$ grows with $n$. At $n=10^4$ the tree is 9 times faster on uniform data. On cut data it is only 1.7 times faster, because the pieces are only about 100 wide, about ten of them share a level and there are far fewer levels to scan. For $n\leq100$ the plain scan is a little faster, because there are only a few levels and the tree costs more to set up.

NFDH and Sleator are the fastest, at about 6 ms for $10^5$ textures; their time is almost only the sorting. On uniform, equalh and heavy data BFDH is the slowest, 1.3 to 1.8 times slower than FFDH, because a balanced search tree is slower than our array-based tree. On cut data, with few levels, the set stays small and BFDH is about as fast as FFDH. All the algorithms handle $10^5$ textures in less than 20 ms.

## Space Complexity

All algorithms store the textures, the sorted order and the output corners, which is $O(n)$. FFDH also stores the tree with fewer than $4n$ entries and two numbers per level, so the total is $O(n)$. BFDH stores one set entry per level and Sleator only a few numbers more, so both are also $O(n)$.

## Factors That Affect the Approximation Ratio

**Number of textures.** This is the strongest factor. In the bound $1.7\cdot OPT+h_{\max}$ the term $h_{\max}$ is fixed while $OPT$ grows with $n$, so the bound gets closer to $1.7\cdot OPT$. In practice FFDH is much better than that: its mean ratio falls from 1.27 to 1.36 at $n=10$ to 1.026 or less at $n=10^4$ on every data type. (On cut data with $n=10$ all pieces are as tall as the strip, so one level is optimal.) With many textures, the short ones at the end of the order fill the gaps at the right end of the lower levels, so only the last few levels are not full.

**Small inputs with tall textures.** The worst real ratio we saw is on cut data with $n=50$: FFDH reaches 1.702 (seed 3, mean 1.650). Here $OPT=500$ and the tallest piece is 448 high, so $h_{\max}$ is almost as large as $OPT$ itself. One level then costs almost a whole $OPT$, and one badly filled level is already a large part of the height. This shows that the $h_{\max}$ term in the bound is real and not only a weakness of the proof. The bound $1.7\cdot500+448=1298$ is still far above the measured height of 851.

**Distribution of heights.** When all heights are equal (equalh), a level wastes nothing above its textures, and FFDH becomes first-fit decreasing bin packing; it reaches 1.003 at $n=10^4$. With uniform heights, every texture shorter than its level's first texture leaves empty space above it, so the ratio is higher (1.026). The heavy type has a few very tall textures; for small $n$ they make tall levels that are hard to fill (1.357 at $n=10$), but for large $n$ there are enough short textures to fill them (1.023 at $n=10^4$).

**Distribution of widths.** Figure 3 shows that narrow textures are easy: with $w_{\max}=200$ or 300 FFDH is within 1.4% of the lower bound. With $w_{\max}=1000$ it rises to 5.6%, because a level that holds only one or two wide textures may leave a gap that no later texture fits. Very narrow textures ($w_{\max}=50$) are a little worse again (4.0%). Here the height, not the width, is the problem: about 40 textures share each level, so each level has many textures much shorter than its first one. Also, the whole packing is low ($LB\approx12{,}800$), so the last, partly filled level of height up to 1000 counts more.

**The lower bound itself.** For the uniform, equalh and heavy types the ratio is measured against $LB$, not $OPT$, so the real ratio may be smaller. On cut data, where $LB=OPT$, FFDH is still within 1.2% at $n=10^4$, which suggests that $LB$ is quite close to $OPT$ for large random inputs.

## Comparison with Other Strip Algorithms

**NFDH.** NFDH is 1.7 to 3 times as fast as FFDH for $n\geq10^4$, but its ratio stays near 1.28 to 1.33 for large $n$ on uniform, equalh and heavy data and does not get better with more textures. The reason is that it never goes back: every level loses the gap at its right end. With uniform widths this gap is large, and the result is close to next-fit bin packing. On cut data NFDH reaches 1.069 at $n=10^5$, while FFDH reaches 1.002. Figure 4 shows the difference: the NFDH levels have large gaps on the right, and FFDH fills them with the short textures from the end.

**BFDH.** BFDH gives almost the same heights as FFDH. In 72 of the 144 inputs behind Figure 2 (4 types, 12 sizes, 3 seeds) its height is a little smaller, in 67 it is the same and in 5 it is larger; the largest gain is about 1% (uniform, $n=1000$: 1.041 against 1.056). However, it is 1.3 to 1.8 times slower in our code on all types except cut. We keep FFDH as our main algorithm because it has the proven $1.7\cdot OPT+h_{\max}$ bound [1] and is simpler and faster, and BFDH gains very little.

**Sleator.** Sleator's algorithm has the best worst-case guarantee, $2.5\cdot OPT$ [2], and it is as fast as NFDH. On small cut inputs it is indeed the safest: at $n=50$ its mean ratio is 1.395, while FFDH reaches 1.650. But for larger inputs its typical ratio is the worst of the four, about 1.32 on uniform data and 1.155 on cut data. Each half is filled next-fit style, so the right end of every half-level is wasted, and textures just over $W/2$ are stacked alone with half of the width empty. Figure 3 shows this clearly: at $w_{\max}=700$ many textures are wider than 500, and Sleator's ratio goes up to 1.494. Figure 4 shows the stacked part and the two uneven halves.

In short, a better worst-case bound does not mean a better typical result. For texture packing, where $n$ is usually large, FFDH gives much lower heights than NFDH and Sleator at a small cost in time.

## Limitations and Improvements

1. We do not rotate textures. If rotation were allowed, we could turn every texture so that it is not taller than it is wide before packing, which usually makes levels lower.
2. Level algorithms waste the space above the shorter textures on each level. A non-level method, such as placing each texture as low and as far left as possible on a skyline, could fill these holes, but it is slower and has no simple bound.
3. For the uniform, equalh and heavy types we only know a lower bound, not $OPT$. An exact solver for very small $n$ would let us measure the real ratio on more kinds of inputs.

# Conclusion

We implemented FFDH for texture packing. With a tree over the free widths of the levels, it runs in $O(n\log n)$ time and packs $10^5$ textures in about 10 ms. Its height is at most $1.7\cdot OPT+h_{\max}$, and in our tests it is far better: within 8% of the lower bound for $n\geq1000$ and within 1.2% of the best height at $n=10^4$ on inputs where we know it. The ratio gets worse when there are few textures and some of them are tall, and when the textures are wide compared with the strip. Compared with other strip algorithms, NFDH and Sleator's algorithm are faster but leave 25 to 30% more height on large random inputs, and BFDH is a little better but slower.

# References

[1] E. G. Coffman Jr., M. R. Garey, D. S. Johnson, and R. E. Tarjan, "Performance bounds for level-oriented two-dimensional packing algorithms," *SIAM Journal on Computing*, vol. 9, no. 4, pp. 808–826, 1980, doi: [10.1137/0209062](https://doi.org/10.1137/0209062).

[2] D. D. K. D. B. Sleator, "A 2.5 times optimal algorithm for packing in two dimensions," *Information Processing Letters*, vol. 10, no. 1, pp. 37–40, 1980, doi: [10.1016/0020-0190(80)90121-0](https://doi.org/10.1016/0020-0190(80)90121-0).

[3] T. H. Cormen, C. E. Leiserson, R. L. Rivest, and C. Stein, *Introduction to Algorithms*, 4th ed. Cambridge, MA, USA: MIT Press, 2022, chs. 2 and 13. [Publisher](https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/).

# Author List

YuJie Sun: code, tests, experiments and report.

YiFeng Zheng: independent reproduction and approval.

# Declaration

We hereby declare that all work in this project, titled “Texture Packing,” is our independent effort as a group.

# Signatures

YuJie Sun

YiFeng Zheng

# Source Code Appendix

The source code is in the `code/` folder of the submission. `code/main.cpp` has all algorithms: `ffdh` (our algorithm, with the tree `LevelTree`), `ffdh_scan`, `nfdh`, `bfdh` and `sleator`, the lower bound `lower_bound_height`, and the timing mode `time_algorithm`; `main` reads the input and runs the chosen algorithm. `code/gen.cpp` is the test data generator. The `README.md` shows how to compile and run them.
