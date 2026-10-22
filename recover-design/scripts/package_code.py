"""Copy validated development sources and reproduction tools into submission snapshots."""
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]


def main():
    """Synchronize only the known files; preserve reports and reviewers' material."""
    for name in ('TA', 'Peer Review'):
        target = ROOT / name
        for relative in ('code/main.cpp', 'scripts/check.py', 'tests/sample1.in', 'problem.md'):
            dest = target / relative
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT / relative, dest)
        (target / 'README.md').write_text('''# Recover the Design — submission snapshot

Run from this directory on Linux / WSL. Requires GCC with C++17 and Python 3.

```bash
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic code/main.cpp -o build/recover-design
./build/recover-design < tests/sample1.in
python3 scripts/check.py --binary build/recover-design
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie code/main.cpp -o build/recover-design-san
python3 scripts/check.py --binary build/recover-design-san
python3 scripts/check.py --binary build/recover-design --benchmark
```

The bundle contains `code/main.cpp`, `scripts/check.py`, `tests/sample1.in`, `problem.md`, this README and the draft report PDF. Executables are created in `build/`; timing runs create `data/timings.json`. Neither generated directory is part of the submission snapshot. The checker accepts any legal solution and independently enumerates small cases. Timing includes process startup, input/output and Python validation.

The exact backtracking solver has exponential worst-case time. These are local tests, not a claim of hidden-judge acceptance. The report is a draft pending independent review. Edit the development directory and run its `make package-code` to refresh these copies.
''')
    print('Updated TA and Peer Review code/checker/sample snapshots.')

if __name__ == '__main__': main()
