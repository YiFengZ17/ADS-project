# Texture Packing — submission snapshot

This bundle contains only the `code/` folder, this README and one report PDF in this directory:

```text
.
├── README.md
├── <report>.pdf
└── code/
    ├── main.cpp   # FFDH (our algorithm), NFDH, BFDH, Sleator and a plain-scan FFDH; also has a timing mode
    └── gen.cpp    # test data generator for the four data types in the report
```

Use Linux, WSL or macOS with a C++17 compiler (GCC or Clang). Run the commands below from the submission directory, keeping the same shell. All generated files go to an external temporary directory.

## Build and run the sample

```bash
ADS_RUN_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ads-p6.XXXXXX")"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/main.cpp -o "$ADS_RUN_DIR/pack"
printf '5 10\n4 3\n6 2\n5 2\n3 1\n10 1\n' > "$ADS_RUN_DIR/sample.in"
"$ADS_RUN_DIR/pack" < "$ADS_RUN_DIR/sample.in"
"$ADS_RUN_DIR/pack" nfdh < "$ADS_RUN_DIR/sample.in"
```

The input is `n W` on the first line (number of textures and strip width), then one `w h` per texture with `1 <= w <= W`. Textures are not rotated. The program prints `height lower_bound` on the first line, then the lower-left corner `x y` of every texture in input order. With no argument it uses FFDH; the other names are `ffdh-scan`, `nfdh`, `bfdh` and `sleator`. For the sample, FFDH prints `6 5` first and NFDH prints `7 5`.

## Sanitizer check

```bash
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer code/main.cpp -o "$ADS_RUN_DIR/pack-san"
"$ADS_RUN_DIR/pack-san" sleator < "$ADS_RUN_DIR/sample.in"
```

The first line should be `6 5`, with no sanitizer messages. This is a small check, not the full test suite reported in the PDF.

## Generate data and time an algorithm

`gen.cpp` takes `n type seed [wmax hmax]` and prints one case with strip width 1000. `type` is `uniform`, `equalh`, `heavy` or `cut` (see the report); for `cut` the best height is exactly `10n`. The same arguments give the same case with any compiler.

`pack ALGORITHM RUNS` is the timing mode. It runs the algorithm once as warmup and then `RUNS` times, timing only the algorithm (sorting included) and not the input reading; a run shorter than 1 ms is repeated and averaged. It prints one line: `algorithm n W height lower_bound median_ms min_ms max_ms`.

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/gen.cpp -o "$ADS_RUN_DIR/gen"
"$ADS_RUN_DIR/gen" 10000 cut 1 > "$ADS_RUN_DIR/cut.in"
"$ADS_RUN_DIR/pack" ffdh 10 < "$ADS_RUN_DIR/cut.in"
"$ADS_RUN_DIR/pack" sleator 10 < "$ADS_RUN_DIR/cut.in"
```

Both lines report the lower bound `100000`, which is the best height for this input. The report's experiments use seeds 1 to 3, one warmup and 10 measured runs per input, and give the median. New timings depend on the machine.

## Full checks

The Python checker (overlap test, Python versions of all algorithms, bound checks) and the scripts that run the experiments and draw the figures are kept in the authors' working directory and are not part of this bundle.
