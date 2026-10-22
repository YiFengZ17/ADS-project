"""Independent geometry checks and exhaustive small-instance feasibility oracle."""
import argparse
import itertools
import json
from pathlib import Path
import random
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]


def encode(grid):
    """Serialize a rectangular case."""
    return f'{len(grid)} {len(grid[0])}\n' + ''.join(' '.join(map(str, row)) + '\n' for row in grid)


def candidates(grid):
    """Find unobstructed pairs by comparing all connector pairs (oracle only)."""
    points = [(r, c) for r, row in enumerate(grid) for c, k in enumerate(row) if k]
    edges = []
    for a, b in itertools.combinations(range(len(points)), 2):
        r, c = points[a]; s, d = points[b]
        if r == s and all(grid[r][x] == 0 for x in range(c + 1, d)):
            edges.append((a, b))
        elif c == d and all(grid[x][c] == 0 for x in range(r + 1, s)):
            edges.append((a, b))
    return points, edges


def crossing(points, first, second):
    """Detect strict interior intersections of two candidate segments."""
    a, b = (points[i] for i in first); c, d = (points[i] for i in second)
    if a[0] != b[0]: a, b, c, d = c, d, a, b
    return a[0] == b[0] and c[1] == d[1] and a[1] < c[1] < b[1] and c[0] < a[0] < d[0]


def oracle(grid):
    """Enumerate all subsets, independently of the solver's propagation and search."""
    points, edges = candidates(grid)
    target = [grid[r][c] for r, c in points]
    conflicts = [(i, j) for i in range(len(edges)) for j in range(i) if crossing(points, edges[i], edges[j])]
    for bits in itertools.product((0, 1), repeat=len(edges)):
        deg = [0] * len(points)
        for bit, (a, b) in zip(bits, edges):
            deg[a] += bit; deg[b] += bit
        if deg == target and not any(bits[i] and bits[j] for i, j in conflicts): return True
    return False


def validate(grid, output, expected):
    """Trace printed rays through cells; check reciprocity, degree, order and crossings."""
    if output == 'No Solution\n':
        assert expected is False, 'unexpected No Solution'
        return
    assert expected is not False, 'solution printed for impossible case'
    points = [(r, c) for r, row in enumerate(grid) for c, k in enumerate(row) if k]
    lines = output.splitlines()
    assert len(lines) == len(points), 'wrong number of connector lines'
    flags = {}
    for point, line in zip(points, lines):
        values = list(map(int, line.split()))
        assert line == ' '.join(map(str, values)), 'invalid spacing'
        assert len(values) == 6 and tuple(values[:2]) == (point[0] + 1, point[1] + 1)
        assert all(v in (0, 1) for v in values[2:])
        assert sum(values[2:]) == grid[point[0]][point[1]], 'wrong degree'
        flags[point] = values[2:]
    occupied = {}
    for (r, c), directions in flags.items():
        for direction, (dr, dc) in enumerate(((-1, 0), (1, 0), (0, -1), (0, 1))):
            if not directions[direction]: continue
            s, d = r + dr, c + dc
            while 0 <= s < len(grid) and 0 <= d < len(grid[0]) and not grid[s][d]:
                orientation = int(dr == 0)
                assert occupied.get((s, d), orientation) == orientation, 'forbidden crossing'
                occupied[s, d] = orientation
                s += dr; d += dc
            assert (s, d) in flags, 'ray ends without connector'
            assert flags[s, d][direction ^ 1] == 1, 'nonreciprocal connection'


def run(binary, grid, expected):
    """Run one solver process with a timeout and validate its output."""
    start = time.perf_counter()
    result = subprocess.run([str(binary)], input=encode(grid), text=True, capture_output=True, timeout=15)
    assert result.returncode == 0, result.stderr
    assert not result.stderr, result.stderr
    validate(grid, result.stdout, expected)
    return (time.perf_counter() - start) * 1000


def large_cases():
    """Build analytically feasible maximum-size designs and one impossible case."""
    n = 50
    return {
        'empty_50': ([[0] * n for _ in range(n)], True),
        'matching_50': ([[1] * n for _ in range(n)], True),
        'full_grid_50': ([[int(r > 0) + int(r < n-1) + int(c > 0) + int(c < n-1) for c in range(n)] for r in range(n)], True),
        'long_segments_50': ([[1] + [0] * (n-2) + [1] for _ in range(n)], True),
        'impossible_50': ([[4] * n for _ in range(n)], False),
    }


def main():
    """Execute differential tests or repeated end-to-end timing runs."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=Path, required=True)
    parser.add_argument('--benchmark', action='store_true')
    args = parser.parse_args(); binary = args.binary.resolve()
    if args.benchmark:
        results = {}
        for name, (grid, expected) in large_cases().items():
            run(binary, grid, expected)  # One warmup process, excluded from measurements.
            results[name] = [run(binary, grid, expected) for _ in range(10)]
        dest = ROOT / 'data/timings.json'; dest.parent.mkdir(exist_ok=True)
        dest.write_text(json.dumps(results, indent=2) + '\n')
        print(json.dumps(results, indent=2)); return
    count = 0
    sample = list(map(int, (ROOT / 'tests/sample1.in').read_text().split()))
    n, m = sample[:2]; grid = [sample[2+r*m:2+(r+1)*m] for r in range(n)]
    run(binary, grid, True); count += 1
    # All possible 2x2 degree grids, including absent connectors and infeasible degrees.
    for values in itertools.product(range(5), repeat=4):
        grid = [list(values[:2]), list(values[2:])]
        run(binary, grid, oracle(grid)); count += 1
    rng = random.Random(17)
    for _ in range(200):
        grid = [[rng.randrange(5) if rng.random() < .7 else 0 for _ in range(3)] for _ in range(3)]
        run(binary, grid, oracle(grid)); count += 1
    # Force two mutually crossing segments without a connector; then allow a degree-4 junction.
    for grid, expected in [([[0,1,0],[1,0,1],[0,1,0]], False),
                           ([[0,1,0],[1,4,1],[0,1,0]], True),
                           ([[1,0,1],[0,0,0],[1,0,1]], True),
                           ([[1,2,1],[0,0,0]], True)]:
        run(binary, grid, expected); count += 1
    # Plant noncrossing solutions on random 4x4 layouts to exercise more satisfiable inputs.
    for _ in range(100):
        layout = [[int(rng.random() < .65) for _ in range(4)] for _ in range(4)]
        points, edges = candidates(layout); chosen = []
        for edge in edges:
            if rng.random() < .5 and not any(crossing(points, edge, other) for other in chosen): chosen.append(edge)
        grid = [[0] * 4 for _ in range(4)]
        for a, b in chosen:
            for p in (a, b):
                r, c = points[p]; grid[r][c] += 1
        run(binary, grid, True); count += 1
    for grid, expected in large_cases().values(): run(binary, grid, expected); count += 1
    # Ensure the validator itself rejects invalid degrees, one-sided links and crossings.
    for grid, output in [([[1,1],[0,0]], '1 1 0 0 0 0\n1 2 0 0 1 0\n'),
                         ([[1,1],[0,0]], '1 1 0 0 0 1\n1 2 0 0 0 1\n'),
                         ([[0,1,0],[1,0,1],[0,1,0]], '1 2 0 1 0 0\n2 1 0 0 0 1\n2 3 0 0 1 0\n3 2 1 0 0 0\n')]:
        try: validate(grid, output, True)
        except AssertionError: pass
        else: raise AssertionError('validator accepted broken solution')
    print(f'PASS: {count} solver cases; 3 validator rejection checks')

if __name__ == '__main__': main()
