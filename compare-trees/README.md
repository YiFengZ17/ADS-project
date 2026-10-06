# Compare Search Trees — Working Project

The ready-to-submit package is `commit-version/`. Its README documents the actual
submitted files and standalone GCC commands for reproducing checks and timings.

## Layout

```text
compare-trees/
├── commit-version/
│   ├── compare-search-trees-draft.pdf
│   ├── README.md
│   └── code/                       # test.c and the five tree implementations
├── report/report.md                # Authoritative editable report content
├── figures/                        # Three figures referenced by Markdown
├── scripts/build_report.py         # Markdown → LaTeX → PDF; never writes Markdown
├── scripts/analyze.py              # Local evidence validation and figure generation
├── Makefile
├── data/                           # Local generated timing evidence and input files
└── build/                          # Local binaries, PDFs and intermediate LaTeX
```

Old `main.tex` and `report/sections/` files are legacy sources and are not used by
`make report` or `make draft`. Do not edit them to change the report.

## Program reproduction

From this working directory, use `make`, `make check`, and `make sanitize`.
Use `make benchmark` to generate fresh local timing data and inputs; this can take
several minutes because sorted BST updates are quadratic. `make analyze` validates
those measurements and regenerates figures without changing report text.
For commands usable with only the submission, see `commit-version/README.md`.

## Markdown → PDF workflow

Requirements: Python 3 with `markdown-it-py`, XeLaTeX with `ctex` and standard LaTeX
packages, TeX Gyre Termes and Latin Modern fonts, and Poppler's `pdftotext`.
`matplotlib` is required only for regenerating figures from fresh benchmarks.

```bash
cd /home/yifeng/projects/ADS/compare-trees
# Edit report/report.md, then:
make report
```

`make draft` is an alias for the same Markdown-based compilation. Neither command
runs the benchmark or the old report generator. The compiler respects ordinary
Markdown line breaks as paragraph breaks, typesets mathematical expressions,
adds a linked table of contents, and uses the typography of `../main.pdf`.
It runs XeLaTeX three times to resolve contents page numbers and links, checks
extracted PDF text against every Markdown item, and verifies that Markdown is unchanged.

The command overwrites `build/main.pdf`, `build/compare-search-trees.pdf`,
`build/compare-search-trees-draft.pdf`, and the existing submission PDF at
`commit-version/compare-search-trees-draft.pdf`. Intermediates and compilation logs
are stored under `build/md-report/`.

PDFs are outputs, never sources for later Markdown edits. Limitations should contain
the three most important points and never more than four. Do not add historical raw
timing files to the submission. Follow `../AGENT.md` for the complete ADS report rules.
