# Shopping With Coupons — submission snapshot

This bundle contains only the `code/` folder, this README and one report PDF in this directory:

```text
.
├── README.md
├── <report>.pdf
└── code/
    ├── main.cpp   # solver: reads one case, prints "items left"; also has a timing mode
    └── gen.cpp    # test data generator for the four data types in the report
```

Use Linux, WSL or macOS with a C++17 compiler (GCC or Clang). Run the commands below from the submission directory, keeping the same shell. All generated files go to an external temporary directory.

## Build and run the sample

```bash
ADS_RUN_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ads-p5.XXXXXX")"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/main.cpp -o "$ADS_RUN_DIR/coupons"
printf '4 30\n12 20 15 10\n9 6 8 7\n' > "$ADS_RUN_DIR/sample.in"
"$ADS_RUN_DIR/coupons" < "$ADS_RUN_DIR/sample.in"
"$ADS_RUN_DIR/coupons" search < "$ADS_RUN_DIR/sample.in"
"$ADS_RUN_DIR/coupons" brute < "$ADS_RUN_DIR/sample.in"
```

The input is `N D` on the first line, then `N` prices and then `N` coupon values. The program prints the largest number of items and the money left. With no argument it uses the heap method; `search` is the binary search method and `brute` is brute force (only for `N <= 5000`). All three lines above should be `8 2`.

## Sanitizer check

```bash
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer code/main.cpp -o "$ADS_RUN_DIR/coupons-san"
"$ADS_RUN_DIR/coupons-san" < "$ADS_RUN_DIR/sample.in"
```

The output should be `8 2` again, with no sanitizer messages. This is a small check, not the full test suite reported in the PDF.

## Generate data and time a method

`gen.cpp` takes `n d type seed` and prints one case. `type` is `random`, `close`, `equal` or `far` (see the report). The same arguments give the same case with any compiler.

`coupons METHOD RUNS` is the timing mode. It runs the method once as warmup and then `RUNS` times, timing only the method (sorting included) and not the input reading. It prints one line: `method n d items left median_ms min_ms max_ms`.

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/gen.cpp -o "$ADS_RUN_DIR/gen"
"$ADS_RUN_DIR/gen" 100000 1000000 equal 1 > "$ADS_RUN_DIR/equal.in"
"$ADS_RUN_DIR/coupons" heap 10 < "$ADS_RUN_DIR/equal.in"
"$ADS_RUN_DIR/coupons" search 10 < "$ADS_RUN_DIR/equal.in"
```

Both lines should report `1000000 0` (items and money left). The report's experiments use seeds 1 to 3, one warmup and 10 measured runs per input, and give the median. New timings depend on the machine.

## Full checks

The comparison with all subsets, the Python sort and the scripts that run the experiments and draw the figures are kept in the authors' working directory and are not part of this bundle. There, the full suite checks 1,639 cases in a release build and 354 cases in an ASan/UBSan build.
