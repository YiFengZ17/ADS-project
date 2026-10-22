"""Reproduce the two failed-branch cases found in the existing seeded test suite."""
import os
from pathlib import Path
import shlex
import subprocess
from check import validate

ROOT = Path(__file__).resolve().parents[1]


def replace_once(source, old, new):
    """Fail on source drift rather than silently producing misleading counters."""
    if source.count(old) != 1:
        raise ValueError('Instrumentation marker changed: ' + old)
    return source.replace(old, new, 1)


def main():
    """Compile a local instrumented copy and verify results and rollback counts."""
    source = (ROOT / 'code/main.cpp').read_text()
    source = replace_once(source, 'using namespace std;', '''using namespace std;
static long long audit_calls = 0, audit_failures = 0, audit_rollbacks = 0, audit_undone = 0;
struct AuditStats {
    ~AuditStats() {
        cerr << "ROLLBACK_STATS " << audit_calls << ' ' << audit_failures << ' '
             << audit_rollbacks << ' ' << audit_undone << '\\n';
    }
} audit_stats;''')
    source = replace_once(source, 'void rollback(size_t mark) {',
                          'void rollback(size_t mark) { ++audit_rollbacks;')
    source = replace_once(source, 'while (trail.size() > mark) {',
                          'while (trail.size() > mark) { ++audit_undone;')
    source = replace_once(source, 'bool search() {', 'bool search() { ++audit_calls;')
    source = replace_once(source, 'if (!propagate()) return false;',
                          'if (!propagate()) { ++audit_failures; return false; }')
    out = ROOT / 'build/rollback-check'
    out.mkdir(parents=True, exist_ok=True)
    instrumented = out / 'instrumented.cpp'
    instrumented.write_text(source)
    binary = out / 'instrumented'
    compiler = shlex.split(os.environ.get('CXX', 'g++'))
    subprocess.run(compiler + ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Wpedantic',
                              str(instrumented), '-o', str(binary)], check=True, timeout=30)
    rows = []
    for name, expected in [('rollback-a', [4, 1, 1, 5]), ('rollback-b', [4, 1, 1, 6])]:
        data = (ROOT / 'tests' / (name + '.in')).read_text()
        values = list(map(int, data.split())); n, m = values[:2]
        grid = [values[2+r*m:2+(r+1)*m] for r in range(n)]
        run = subprocess.run([str(binary)], input=data, text=True, capture_output=True, timeout=3)
        assert run.returncode == 0, run.stderr
        validate(grid, run.stdout, True)
        fields = run.stderr.split()
        assert fields and fields[0] == 'ROLLBACK_STATS', run.stderr
        counts = list(map(int, fields[1:]))
        assert counts == expected, (name, counts, expected)
        rows.append(name + ': calls, propagation failures, rollbacks, undone edges = ' + str(counts))
    result = '\n'.join(rows) + '\nPASS: 2 existing-suite rollback cases\n'
    (out / 'results.txt').write_text(result)
    print(result, end='')


if __name__ == '__main__':
    main()
