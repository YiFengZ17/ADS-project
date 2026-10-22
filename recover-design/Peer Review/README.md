# Recover the Design — submission snapshot

Run from this directory on Linux / WSL. Requires GCC with C++17 and Python 3.

```bash
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/main.cpp -o build/recover-design
./build/recover-design < tests/sample1.in
python3 scripts/check.py --binary build/recover-design
python3 scripts/check_rollback.py
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie code/main.cpp -o build/recover-design-san
python3 scripts/check.py --binary build/recover-design-san
python3 scripts/check.py --binary build/recover-design --benchmark
```

The bundle contains `code/main.cpp`, both check scripts, the sample and two rollback inputs in `tests/`, `problem.md`, this README and the draft report PDF. Executables are created in `build/`; timing runs create `data/timings.json`. Neither generated directory is part of the submission snapshot. The checker accepts any legal solution and independently enumerates small cases. Timing includes process startup, input/output and Python validation. The rollback script instruments a local source copy without changing solver decisions, and writes counts to `build/rollback-check/results.txt`. Its two fixed inputs come from the existing 935-case suite; they each exercise one failed branch, not deeper failed searches.

The exact backtracking solver has exponential worst-case time. These are local tests, not a claim of hidden-judge acceptance. The report is a draft pending independent review. Edit the development directory and run its `make package-code` to refresh these copies.
