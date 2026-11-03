# Recover the Design — submission snapshot

This bundle contains only the `code/` folder, this README and one report PDF in this directory:

```text
.
├── README.md
├── <report>.pdf
└── code/
    ├── main.cpp   # the solver: reads one grid, prints a design or "No Solution"
    └── gen.cpp    # generator for the large test inputs used in the report
```

Use Linux / WSL and a C++17 compiler. Run the commands below from the submission directory, keeping the same shell. All generated files go to an external temporary directory.

## Build and run the sample

```bash
ADS_RUN_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ads-p3.XXXXXX")"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/main.cpp -o "$ADS_RUN_DIR/recover-design"
cat > "$ADS_RUN_DIR/sample.in" <<'INPUT'
5 6
0 0 2 0 0 2
1 3 3 1 0 0
2 2 1 0 0 2
0 0 0 0 0 1
2 0 2 0 0 2
INPUT
"$ADS_RUN_DIR/recover-design" < "$ADS_RUN_DIR/sample.in" > "$ADS_RUN_DIR/sample.out"
cat "$ADS_RUN_DIR/sample.out"
```

The program reads one case and prints connector directions or `No Solution`. Several valid outputs may exist. `echo "$ADS_RUN_DIR"` shows the executable and sample output location.

## Small correctness and sanitizer checks

The following case has a unique solution: two degree-1 connectors joined horizontally.

```bash
printf '2 2\n1 1\n0 0\n' > "$ADS_RUN_DIR/pair.in"
printf '1 1 0 0 0 1\n1 2 0 0 1 0\n' > "$ADS_RUN_DIR/pair.expected"
"$ADS_RUN_DIR/recover-design" < "$ADS_RUN_DIR/pair.in" > "$ADS_RUN_DIR/pair.out"
diff -u "$ADS_RUN_DIR/pair.expected" "$ADS_RUN_DIR/pair.out"
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie code/main.cpp -o "$ADS_RUN_DIR/recover-design-san"
"$ADS_RUN_DIR/recover-design-san" < "$ADS_RUN_DIR/sample.in" > "$ADS_RUN_DIR/sample-san.out"
```

The diff should be empty, and the sanitizer run should produce no diagnostics. These are small checks, not the full test suite reported in the PDF.

## Generate large inputs

`gen.cpp` takes `n m density type seed` and prints one grid in the input format. `type` is `planted` (random fences that never cross, so a solution surely exists), `broken` (a planted grid with one unit of degree moved to another connector) or `random` (random degrees from 1 to 4). The same arguments always give the same grid.

```bash
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/gen.cpp -o "$ADS_RUN_DIR/gen"
"$ADS_RUN_DIR/gen" 50 50 1.0 planted 1 > "$ADS_RUN_DIR/planted-50.in"
time "$ADS_RUN_DIR/recover-design" < "$ADS_RUN_DIR/planted-50.in" > "$ADS_RUN_DIR/planted-50.out"
```

The report's timing table uses n by n grids for n = 10, 20, 30, 40, 50, seeds 1 to 5, and the kinds `planted 1.0`, `planted 0.5`, `broken 1.0` and `random 0.6`. Each input runs once as warmup and then 10 times; the report gives medians. New timings depend on the machine.

## Full checks

The output checker, the brute-force comparison and the timing scripts named in the report are kept in the authors' working directory and are not part of this bundle. There, the full suite checks 1,170 grid cases, three broken outputs that the checker must reject, and 2,000 Boolean formulas against full truth tables, in both a release build and an ASan/UBSan build.

The solver is self-contained C++17 and uses degree-sum checks, watched propagation and conflict learning. Worst-case time and learned-clause storage can grow exponentially. Local checks do not prove that the hidden judge tests pass.
