# Recover the Design — submission snapshot

This bundle contains only `code/main.cpp`, this README and one report PDF in this directory. Use Linux / WSL and a C++17 compiler. Run the commands below from the submission directory, keeping the same shell. All generated files go to an external temporary directory.

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

## Full checks and timing reproduction

The report's paths `scripts/check.py`, `scripts/check_rollback.py` and `tests/` refer to the project development directory. Those tools and inputs are not part of this clean submission bundle. With the full project checkout, run the following from `recover-design/`:

```bash
make check
python3 scripts/check_rollback.py
make sanitize
make benchmark
```

The development workflow checks 935 solver cases and three invalid outputs; the rollback check reruns two cases from that suite. It writes executables to the project's `build/`, rollback counts to `build/rollback-check/results.txt`, and timing measurements to `data/timings.json`. Timing includes process startup, input/output and Python validation. These outputs stay outside TA and Peer Review.

The exact backtracking solver has exponential worst-case time. Local checks do not establish hidden-judge acceptance.
