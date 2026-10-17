"""Exercise DIMACS parser failures through the released benchmark CLI."""
from pathlib import Path
import os
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BINARY = Path(os.environ.get('BENCHMARK', ROOT / 'build/benchmark')).resolve()
CASES = [
    ('missing header', 'c comments only\n', 'missing DIMACS header'),
    ('duplicate header', 'p sp 2 1\np sp 2 1\na 1 2 7\n', 'duplicate DIMACS header'),
    ('truncated arc', 'p sp 2 0\na 1 2\n', 'malformed arc record'),
    ('arc before header', 'a 1 2 7\np sp 2 1\n', 'arc before DIMACS header'),
    ('negative arc count', 'p sp 2 -1\n', 'invalid DIMACS header'),
    ('truncated header', 'p sp 2\n', 'invalid DIMACS header'),
]


def main():
    with tempfile.TemporaryDirectory() as directory:
        graph = Path(directory) / 'input.gr'
        for label, content, message in CASES:
            graph.write_text(content)
            run = subprocess.run([str(BINARY), str(graph), '1000', '42'],
                                 capture_output=True, text=True)
            if run.returncode == 0 or message not in run.stderr:
                raise AssertionError(f'{label}: expected {message!r}, got exit={run.returncode}, stderr={run.stderr!r}')
        graph.write_text('c valid\np sp 2 1\na 1 2 7\n')
        run = subprocess.run([str(BINARY), str(graph), '1000', '42'],
                             capture_output=True, text=True)
        if run.returncode != 0 or not run.stdout.startswith('method,query,'):
            raise AssertionError(f'valid graph rejected: {run.stderr!r}')
    print('PASS: parser rejects missing/duplicate headers and truncated arcs')


if __name__ == '__main__':
    main()
