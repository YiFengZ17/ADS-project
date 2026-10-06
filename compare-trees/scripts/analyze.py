"""Validate fresh local timing CSV and compute medians/ranges without editing reports."""
from pathlib import Path
import csv
import math
import statistics
from collections import defaultdict

root = Path(__file__).resolve().parents[1]
source = root / 'data/results/raw.csv'
if not source.exists():
    raise SystemExit('No local timings. Run make benchmark first.')
groups = defaultdict(list)
with source.open(newline='') as f:
    for row in csv.DictReader(f):
        key = (row['scenario'], int(row['n']), row['tree'])
        values = [float(row[k]) for k in ('insert_seconds', 'delete_seconds', 'total_seconds')]
        if any(not math.isfinite(v) or v < 0 for v in values):
            raise SystemExit(f'Invalid timing in {key}')
        if not math.isclose(values[0]+values[1], values[2], abs_tol=2e-8, rel_tol=1e-5):
            raise SystemExit(f'Inconsistent total in {key}')
        repeat = int(row['repeat'])
        if any(item[0] == repeat for item in groups[key]):
            raise SystemExit(f'Duplicate repeat in {key}')
        groups[key].append((repeat, values))
expected = {(s, n, t) for s in ('increasing_same','increasing_reverse','random_random')
            for n in (1000,2000,5000,10000,20000,50000,100000)
            for t in ('BST','AVL','Splay','RedBlack','BPlus32')}
if set(groups) != expected or any(len(v) != 3 for v in groups.values()):
    raise SystemExit('Expected 105 scenario/size/tree groups with three distinct repeats each.')
destination = source.with_name('summary.csv')
with destination.open('w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['scenario','n','tree','phase','median_ms','min_ms','max_ms'])
    for key, runs in sorted(groups.items()):
        for i, phase in enumerate(('insert','delete','total')):
            values = [run[1][i]*1000 for run in runs]
            writer.writerow([*key, phase, statistics.median(values), min(values), max(values)])
print(f'Validated 315 measurements; wrote {destination}. Report text and figures are unchanged.')
