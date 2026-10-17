# Project 2 reproduction

Run every command below from **this submission directory**. It contains `Makefile`, `code/main.cpp`, `scripts/analyze.py`, and `scripts/test_parser.py`. The PDF is the submitted report. C++17, Python 3, `curl`, `gzip`, and matplotlib are needed for the full experiment; Clang or GCC sanitizers are needed for `make sanitizer`.

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

`make` writes `build/benchmark`. The tests print PASS or exit nonzero. Each benchmark CSV contains 1,000 matching source-target queries for each heap, with distances and seconds per query. A mismatch causes failure. `scripts/analyze.py` checks the paired answers and writes `data/summary.csv`, `data/distribution.csv`, `figures/runtime-vs-size.pdf`, and `figures/runtime-vs-size.png`. These are **new local results**, not the historical CSVs from the report. Large road files, raw timing CSVs, build files, and caches are not bundled.

The download links above are the official 9th DIMACS distance graph files. The report PDF already includes the historical summary table and figure. Rebuilding the PDF from Markdown is an author workflow in the development directory, not a prerequisite for running the submitted code.
