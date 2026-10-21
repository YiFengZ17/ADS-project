"""Check ten-run timing CSV and write medians and ranges; leave reports unchanged."""
from pathlib import Path
import csv
import math
import statistics
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[1]
SCENARIOS = ('increasing_same', 'increasing_reverse', 'random_random')
SIZES = (1000, 2000, 5000, 10000, 20000, 50000, 100000)
TREES = ('BST', 'AVL', 'Splay', 'RedBlack', 'BPlus32')
PHASES = ('insert', 'delete', 'total')
REPEATS = 10


def validate(rows):
    """Reject missing runs, warmups, duplicates, bad times, and mismatched inputs."""
    groups = defaultdict(list)
    seeds = {}
    for row in rows:
        key = (row['scenario'], int(row['n']), row['tree'])
        values = [float(row[phase + '_seconds']) for phase in PHASES]
        if any(not math.isfinite(v) or v <= 0 for v in values):
            raise ValueError(f'Invalid timing in {key}')
        if not math.isclose(values[0] + values[1], values[2], abs_tol=2e-8, rel_tol=1e-5):
            raise ValueError(f'Inconsistent total in {key}')
        repeat = int(row['repeat'])
        if repeat not in range(REPEATS):
            raise ValueError(f'Unexpected repeat {repeat} in {key}')
        if any(item[0] == repeat for item in groups[key]):
            raise ValueError(f'Duplicate repeat in {key}')
        trial = (*key[:2], repeat)
        seed = int(row['seed'])
        expected_seed = (0x9e3779b9 ^ key[1] ^ ((repeat + 1) * 0x85ebca6b)) & 0xffffffff
        if seed != expected_seed or seeds.setdefault(trial, seed) != seed:
            raise ValueError(f'Unexpected seed in {key}')
        groups[key].append((repeat, values))
    expected = {(s, n, t) for s in SCENARIOS for n in SIZES for t in TREES}
    if set(groups) != expected or any(len(v) != REPEATS for v in groups.values()):
        raise ValueError('Expected 105 groups with repeats 0 through 9 in each.')
    return groups


def summarize(groups):
    """Compute each phase separately, including the recorded total time."""
    for key, runs in sorted(groups.items()):
        for i, phase in enumerate(PHASES):
            values = [run[1][i] * 1000 for run in runs]
            yield [*key, phase, statistics.median(values), min(values), max(values)]


def main():
    """Read the local benchmark output and save its checked summary."""
    source = ROOT / 'data/results/raw.csv'
    with source.open(newline='') as f:
        groups = validate(csv.DictReader(f))
    destination = source.with_name('summary.csv')
    with destination.open('w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['scenario', 'n', 'tree', 'phase', 'median_ms', 'min_ms', 'max_ms'])
        writer.writerows(summarize(groups))
    print(f'Validated 1,050 measured runs; wrote {destination}. Report and figures unchanged.')


if __name__ == '__main__':
    main()
