"""Compare the actual Boolean engine against exhaustive truth tables."""
import argparse
import itertools
import os
from pathlib import Path
import random
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    """Compile a harness around the submitted engine and check satisfiability/models."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    out = ROOT / 'build' / ('boolean-check-san' if args.sanitize else 'boolean-check')
    out.mkdir(parents=True, exist_ok=True)
    source = out / 'harness.cpp'
    source.write_text('''#define main project_main
#include "../../code/main.cpp"
#undef main
int main() {
    int tests; std::cin >> tests;
    while (tests--) {
        int n, m; std::cin >> n >> m; BooleanSolver solver(n);
        while (m--) {
            int size; std::cin >> size; std::vector<int> clause(size);
            for (int &x : clause) std::cin >> x;
            solver.add_clause(clause);
        }
        if (!solver.solve()) std::cout << "UNSAT\\n";
        else {
            std::cout << "SAT";
            for (int i = 0; i < n; ++i) std::cout << ' ' << solver.selected(i);
            std::cout << '\\n';
        }
    }
}
''')
    binary = out / 'harness'
    flags = ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Wpedantic']
    if args.sanitize:
        flags += ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    subprocess.run(shlex.split(os.environ.get('CXX', 'c++')) + flags +
                   [str(source), '-o', str(binary)], check=True, timeout=30)
    rng = random.Random(20261101)
    cases = []
    for index in range(2000):
        n = rng.randrange(1, 9)
        clauses = []
        for _ in range(rng.randrange(1, 45)):
            variables = rng.sample(range(n), rng.randrange(1, min(n, 4) + 1))
            clauses.append([2 * v + rng.randrange(2) for v in variables])
        if index % 53 == 0:
            clauses.append([])
        cases.append((n, clauses))
    data = str(len(cases)) + '\n' + ''.join(
        f'{n} {len(clauses)}\n' + ''.join(
            str(len(clause)) + ' ' + ' '.join(map(str, clause)) + '\n'
            for clause in clauses) for n, clauses in cases)
    result = subprocess.run([str(binary)], input=data, text=True, capture_output=True, timeout=30)
    assert result.returncode == 0 and not result.stderr, result.stderr
    lines = result.stdout.splitlines()
    assert len(lines) == len(cases)
    for index, ((n, clauses), line) in enumerate(zip(cases, lines)):
        def satisfies(values):
            return all(any(bool(values[p // 2]) != bool(p % 2) for p in clause)
                       for clause in clauses)
        expected = any(satisfies(values) for values in itertools.product((0, 1), repeat=n))
        assert (line != 'UNSAT') == expected, (index, clauses, line, expected)
        if expected:
            values = list(map(int, line.split()[1:]))
            assert len(values) == n and all(v in (0, 1) for v in values)
            assert satisfies(values), (index, line)
    print('PASS: 2000 Boolean formulas against exhaustive truth tables')


if __name__ == '__main__':
    main()
