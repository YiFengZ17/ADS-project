"""Check learning, backtracking and restarts on a temporary instrumented solver."""
import argparse
import os
from pathlib import Path
import shlex
import subprocess
from check import candidates, encode, oracle, planted, validate

ROOT = Path(__file__).resolve().parents[1]


def replace_once(source, old, new):
    """Reject stale instrumentation markers instead of reporting false coverage."""
    if source.count(old) != 1:
        raise ValueError('Instrumentation marker changed: ' + old)
    return source.replace(old, new, 1)


def main():
    """Instrument a copy, check legal outputs, and require actual learned backtracking."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    args = parser.parse_args()
    source = (ROOT / 'code/main.cpp').read_text()
    source = replace_once(source, 'using namespace std;', '''using namespace std;
static long long audit_conflicts = 0, audit_learned = 0, audit_backtracks = 0;
static long long audit_undone = 0, audit_jumps = 0, audit_restarts = 0;
struct AuditStats {
    ~AuditStats() {
        cerr << "SEARCH_STATS " << audit_conflicts << ' ' << audit_learned << ' '
             << audit_backtracks << ' ' << audit_undone << ' ' << audit_jumps << ' '
             << audit_restarts << '\\n';
    }
} audit_stats;''')
    for old, new in [
        ('if (conflict >= 0) {', 'if (conflict >= 0) { ++audit_conflicts;'),
        ('vector<int> learned = analyze(conflict, back_level);',
         'vector<int> learned = analyze(conflict, back_level); ++audit_learned;'),
        ('size_t mark = boundaries[level];',
         '++audit_backtracks; audit_jumps += ((int)boundaries.size() - level > 1);\n        size_t mark = boundaries[level]; audit_undone += trail.size() - mark;'),
        ('backtrack(0); conflicts_since_restart = 0;',
         '++audit_restarts; backtrack(0); conflicts_since_restart = 0;'),
    ]:
        source = replace_once(source, old, new)
    out = ROOT / 'build' / ('rollback-check-san' if args.sanitize else 'rollback-check')
    out.mkdir(parents=True, exist_ok=True)
    instrumented = out / 'instrumented.cpp'; instrumented.write_text(source)
    binary = out / 'instrumented'
    compiler = shlex.split(os.environ.get('CXX', 'c++'))
    flags = ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Wpedantic']
    if args.sanitize:
        flags += ['-O1', '-g', '-fsanitize=address,undefined', '-fno-omit-frame-pointer']
    subprocess.run(compiler + flags + [str(instrumented), '-o', str(binary)],
                   check=True, timeout=30)
    cases = []
    for name in ('rollback-a', 'rollback-b', 'learning-unsat'):
        values = list(map(int, (ROOT / 'tests' / (name + '.in')).read_text().split()))
        n, m = values[:2]
        cases.append((name, [values[2+r*m:2+(r+1)*m] for r in range(n)]))
    cases.append(('learning-50', planted(50, 1.0, 17)))
    rows = ['Counts: conflicts, learned clauses, backtracks, undone assignments, multi-level jumps, restarts']
    for name, grid in cases:
        expected = name != 'learning-unsat'
        if not expected:
            # Bound independent enumeration and verify the fixture, not solver output.
            assert len(candidates(grid)[1]) <= 20
            assert oracle(grid) is False
        result = subprocess.run([str(binary)], input=encode(grid), text=True,
                                capture_output=True, timeout=3)
        assert result.returncode == 0, result.stderr
        validate(grid, result.stdout, expected)
        fields = result.stderr.split()
        assert fields[0] == 'SEARCH_STATS' and len(fields) == 7, result.stderr
        counts = list(map(int, fields[1:]))
        if name == 'learning-50':
            assert all(count > 0 for count in counts), counts
        if name == 'learning-unsat':
            # Require a learned conflict, a backtrack and the final contradiction.
            assert counts[0] >= 2 and counts[1] > 0 and counts[2] > 0, counts
        rows.append(f'{name}: {counts}')
    rows.append('PASS: 4 search-path cases; exhaustive UNSAT oracle verified')
    text = '\n'.join(rows) + '\n'
    (out / 'results.txt').write_text(text); print(text, end='')


if __name__ == '__main__':
    main()
