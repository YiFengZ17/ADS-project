# Compare Search Trees

This submission compares five C implementations of search trees under ordered
and random insertion/deletion sequences. It includes the experimental report,
complete source code, and instructions for reproducing the checks and timings.

## Submission layout

```text
TA/
├── yds_2026G17_P1.pdf  # Experimental report
├── README.md                     # Reproduction instructions
└── code/
    ├── test.c                    # Shared definitions, smoke test and benchmark driver
    ├── bst.c                     # Unbalanced binary search tree
    ├── avl.c                     # AVL tree
    ├── splay.c                   # Top-down splay tree
    ├── redblack.c                # Left-leaning red-black tree
    └── bplus.c                   # B+ tree with capacity 32
```

The submitted report is `yds_2026G17_P1.pdf`.

## Requirements

GCC with C11 support and the standard C library is required. The benchmark also
uses POSIX `clock_gettime(CLOCK_MONOTONIC)` and `mkdir`, so the compiler environment
must provide these interfaces. The commands below run directly on Linux;
other systems require a compatible toolchain. On Windows, compatibility depends
on the GCC distribution and its runtime.

The optional sanitizer check requires AddressSanitizer and UndefinedBehaviorSanitizer
support. Python and LaTeX are not needed to run the submitted code.

Run all commands below from `TA/`.

The submission contains only `code/`, this README and its report PDF. All commands write executables and data to an external temporary directory. Keep the same shell for the commands below; `echo "$ADS_RUN_DIR"` shows the output location.

## Build and smoke test

```bash
ADS_RUN_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ads-p1.XXXXXX")"
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic code/test.c -o "$ADS_RUN_DIR/compare-trees"
"$ADS_RUN_DIR/compare-trees" --check
```

Compile only `test.c`, which includes the five tree implementations. Do not compile
all six `.c` files as separate translation units.

The smoke test checks deletion from an empty tree, boundary keys, insertion of
1,000 ordered keys, sampled membership, and complete deletion. Successful output
reports `PASS` for all five trees. It is not an exhaustive structural-invariant test.

## Optional sanitizer check

Using the same shell and `ADS_RUN_DIR` from the build step, run:

```bash
gcc -std=c11 -O1 -g -Wall -Wextra -Wpedantic \
    -fsanitize=address,undefined -fno-omit-frame-pointer \
    code/test.c -o "$ADS_RUN_DIR/compare-trees-sanitize"
"$ADS_RUN_DIR/compare-trees-sanitize" --check
```

Expect five `PASS` results and no sanitizer diagnostics.

## Reproduce the benchmark

```bash
mkdir -p "$ADS_RUN_DIR/inputs"
"$ADS_RUN_DIR/compare-trees" --bench "$ADS_RUN_DIR/results.csv" "$ADS_RUN_DIR/inputs"
```

The driver runs all five trees at N = 1,000, 2,000, 5,000, 10,000, 20,000, 50,000,
and 100,000, with one warmup and ten measured runs for each scenario:

1. Increasing insertion and increasing deletion.
2. Increasing insertion and decreasing deletion.
3. Independently shuffled insertion and deletion.

Random inputs use Fisher-Yates shuffles driven by xorshift32. Seeds depend on N
and the repetition number; the saved input files include the seed and both orders.

Within each trial, all trees receive the same inputs. The run generates 1,050 timing
rows in `$ADS_RUN_DIR/results.csv` and 231 input files in `$ADS_RUN_DIR/inputs/` (21 warmup
inputs and 210 measured inputs). Warmups use repeat -1 and are excluded from the CSV. Trial progress is
printed to standard error. Ordered BST runs can take several minutes because their
total cost is quadratic. Repeating the command overwrites the generated measurements.

The CSV columns are:

```text
scenario,n,repeat,seed,tree,insert_seconds,delete_seconds,total_seconds
```

Each input file contains trial metadata, N, the insertion order, and the deletion
order. Insertion and deletion are timed separately. Allocation and freeing within
tree operations are included; input generation, membership checks, and file output
are excluded.

## Compare with the report

For each scenario, N, and tree, take the median of the ten `total_seconds` values
and multiply by 1,000 to obtain total milliseconds. Compute insertion and deletion
medians separately for the phase plots. Error bars represent the minimum and maximum
of the ten measured runs.

New timings will vary with hardware, compiler, and system load. These commands
reproduce the experiment and its checks, not the exact historical timing values.
The submission does not include the Markdown sources, figures, or compiler needed
to rebuild the report PDF.

The report uses short explanations of update steps and performance differences.
The timing tables and plots use a fresh ten-run experiment on 2026-10-21.
The tree code is unchanged; the driver now runs one full warmup per configuration.
