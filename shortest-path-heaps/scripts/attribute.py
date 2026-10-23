"""Run the attribution experiment and summarize where the queue gap comes from.

The main benchmark answers "which queue is faster". This script answers "why",
by splitting each query into initialization and search, counting the work each
queue performs, and adding an indexed binary heap as a third reference point.

Usage:
    python3 scripts/attribute.py graph.gr [queries] [label]

The output is a Markdown table ready to paste into the report's analysis
section. Run it once per graph and keep the printed table.
"""
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / 'build/benchmark'

# Rows of the report table, in order, with the label used in the report.
FIELDS = [
    ('ratio_fibonacci_over_binary', 'Fibonacci / lazy binary'),
    ('ratio_indexed_over_binary', 'Indexed binary / lazy binary'),
    ('gap_initialization_share', 'Share of gap from initialization'),
    ('gap_search_share', 'Share of gap from search'),
    ('fib_consolidate_share', 'Share of Fibonacci time in consolidate()'),
    ('fib_consolidations', 'consolidate() calls'),
    ('fib_pushes', 'Fibonacci pushes'),
    ('fib_pops', 'Fibonacci extract-min calls'),
    ('fib_decrease_key', 'Fibonacci decrease-key calls'),
    ('fib_links', 'Fibonacci links'),
    ('fib_root_ring_avg', 'Average root-ring size'),
    ('binary_pushes', 'Lazy binary pushes'),
    ('binary_pops', 'Lazy binary extract-min calls'),
    ('binary_stale_pops', 'Lazy binary stale pops'),
]


def run(graph, queries):
    """Run the benchmark attribution mode and parse its key/value output."""
    result = subprocess.run(
        [str(BINARY), '--attribute', str(graph), str(queries)],
        capture_output=True, text=True, check=True)
    values = {}
    header = ''
    for line in result.stdout.splitlines():
        if line.startswith('dataset='):
            header = line
        elif ',' in line:
            key, _, value = line.partition(',')
            try:
                values[key] = float(value)
            except ValueError:
                pass
    return header, values


def main():
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    graph = Path(sys.argv[1])
    queries = sys.argv[2] if len(sys.argv) > 2 else '1000'
    label = sys.argv[3] if len(sys.argv) > 3 else graph.stem

    header, values = run(graph, queries)
    print(f'### {label}\n')
    print(header)
    print()
    print('| Quantity | Value |')
    print('| --- | ---: |')
    for key, name in FIELDS:
        if key not in values:
            continue
        value = values[key]
        text = f'{value:,.0f}' if abs(value) >= 1000 or value == int(value) else f'{value:.4g}'
        print(f'| {name} | {text} |')
    print()
    print('| Phase | Init (ms) | Search (ms) | Total (ms) |')
    print('| --- | ---: | ---: | ---: |')


if __name__ == '__main__':
    main()
