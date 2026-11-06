# Improving Dynamic Programming for Knapsack — submission snapshot

This bundle contains only the `code/` folder, this README and one report PDF in this directory:

```text
.
├── README.md
├── <report>.pdf
└── code/
    ├── knapsack.h     # shared types and the list of algorithms
    ├── knapsack.cpp   # DP-1, DP-with-Lists, DP-with-Upper-Bounds and the small-weight method
    ├── main.cpp       # solver: reads one instance, prints the optimal value
    ├── bench.cpp      # performance test: times one algorithm on one instance file
    └── gen.cpp        # test data generator for the five instance types in the report
```

Use Linux, WSL or macOS with a C++17 compiler (GCC or Clang; the code uses `__int128`). Run the commands below from the submission directory, keeping the same shell. All generated files go to an external temporary directory.

## Build and run a small example

```bash
ADS_RUN_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ads-p4.XXXXXX")"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/main.cpp code/knapsack.cpp -o "$ADS_RUN_DIR/knapsack"
printf '7 9\n2 6\n3 5\n6 8\n7 9\n5 6\n9 7\n4 3\n' > "$ADS_RUN_DIR/book.in"
"$ADS_RUN_DIR/knapsack" all < "$ADS_RUN_DIR/book.in"
```

The input is `n c` on the first line and then one line `w p` (weight and profit) per item. `knapsack NAME` prints only the optimal value of one algorithm, where `NAME` is `dp1`, `lists`, `lists-sorted`, `bounds` or `small`; `knapsack all` prints `NAME value` for all five. The example above is the instance of Fig. 3.7 in the textbook, and every line should end with `15`. An algorithm that does not accept an input prints `-` in the `all` mode: `dp1` needs `c <= 2*10^8` for its array, and `small` needs the largest usable weight to be at most 2000.

## Sanitizer check

```bash
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer code/main.cpp code/knapsack.cpp -o "$ADS_RUN_DIR/knapsack-san"
"$ADS_RUN_DIR/knapsack-san" all < "$ADS_RUN_DIR/book.in"
```

The output should be the same as above, with no sanitizer messages. This is a small check, not the full test suite reported in the PDF.

## Generate data and time an algorithm

`gen.cpp` takes `n R type seed [ratio]` and prints one instance. `type` is `uncorrelated`, `weak`, `strong`, `inverse` or `subset`; weights are about 1 to `R`, and the capacity is `ratio` (default 0.5) times the total weight. The same arguments always give the same instance.

`bench.cpp` takes `algorithm file [runs]`. It runs the algorithm once as warmup and then `runs` times (default 10), timing only the algorithm and not the input reading. It prints one line: `algorithm n c wmax value work median_ms min_ms max_ms`, where `work` is the number of DP cells (dp1), list states (list methods) or convolution entries (small).

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/gen.cpp -o "$ADS_RUN_DIR/gen"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/bench.cpp code/knapsack.cpp -o "$ADS_RUN_DIR/bench"
"$ADS_RUN_DIR/gen" 1000 1000 strong 1 > "$ADS_RUN_DIR/strong.in"
"$ADS_RUN_DIR/bench" dp1 "$ADS_RUN_DIR/strong.in"
"$ADS_RUN_DIR/bench" bounds "$ADS_RUN_DIR/strong.in"
```

The report's experiments use the five types, seeds 1 to 3, one warmup and 10 measured runs per input, and give the median. The sizes of each experiment are listed in the report. New timings depend on the machine.

## Full checks

The brute-force comparison, the independent Python DP and the scripts that run all experiments and draw the figures are kept in the authors' working directory and are not part of this bundle. There, the full suite checks 3,312 cases in a release build and 586 cases in an ASan/UBSan build.
