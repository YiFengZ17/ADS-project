# Project 2 reproduction

This bundle contains only `code/main.cpp`, this README and one report PDF: `yds_2026G17_P2.pdf` in TA, or `shortest-path-heaps.pdf` in Peer Review. Run the commands below from the submission directory on Linux / WSL with a C++17 compiler. All generated files go to an external temporary directory; keep the same shell for all steps.

## Build and correctness check

```bash
ADS_RUN_DIR="$(mktemp -d "${TMPDIR:-/tmp}/ads-p2.XXXXXX")"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/main.cpp -o "$ADS_RUN_DIR/benchmark"
"$ADS_RUN_DIR/benchmark" --self-test
```

The built-in test checks edge cases and 2,000 random source-target pairs. Success prints `PASS: edge cases and 2000 random pairs`. Use `echo "$ADS_RUN_DIR"` to locate the output directory.

## Sanitizer check

```bash
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie code/main.cpp -o "$ADS_RUN_DIR/benchmark-san"
"$ADS_RUN_DIR/benchmark-san" --self-test
```

Expect the same PASS message and no sanitizer diagnostics.

## Reproduce the timing runs

Obtain the NY, BAY and COL distance graphs from the 9th DIMACS distance graph collection listed in the report. Place the decompressed files at `$ADS_RUN_DIR/NY.gr`, `$ADS_RUN_DIR/BAY.gr` and `$ADS_RUN_DIR/COL.gr`. Graphs and download tools are not included in this bundle.

```bash
for region in NY BAY COL; do
  "$ADS_RUN_DIR/benchmark" --validate "$ADS_RUN_DIR/$region.gr"
  "$ADS_RUN_DIR/benchmark" "$ADS_RUN_DIR/$region.gr" 1000 20261016 > "$ADS_RUN_DIR/results-$region.csv"
done
```

Each CSV records 1,000 matching source-target queries for each heap, with distances and seconds per query. A distance mismatch causes failure. These are new local results, not the historical measurements in the PDF.

## Development-only tools

The parser test, analysis scripts, Makefile, data and report sources are kept in the project development directory, outside this bundle. Paths to these tools in the report refer to that directory. With the full project checkout, run `make self-test parser-test`, `make sanitizer`, and the timing and analysis commands in the project-level README from `shortest-path-heaps/`. That workflow writes to the project's `build/`, `data/` and `figures/`, never inside TA or Peer Review. The report PDF can be read without these tools.
