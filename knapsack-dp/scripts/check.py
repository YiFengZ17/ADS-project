"""Correctness tests: compare every algorithm with brute force and an independent Python DP."""
import argparse
from pathlib import Path
import random
import subprocess

ROOT = Path(__file__).resolve().parents[1]
ALGORITHMS = ('dp1', 'lists', 'lists-sorted', 'bounds', 'small')


def instance_text(c, items):
    """Write an instance in the input format: "n c" and then "w p" lines."""
    return f'{len(items)} {c}\n' + ''.join(f'{w} {p}\n' for w, p in items)


def run_all(binary, text):
    """Run "knapsack all" and return {algorithm: value or None if it refused the input}."""
    result = subprocess.run([str(binary), 'all'], input=text, capture_output=True, text=True, timeout=300)
    assert result.returncode == 0 and not result.stderr, result.stderr
    values = {}
    for line in result.stdout.split('\n'):
        if line:
            name, value = line.split()
            values[name] = None if value == '-' else int(value)
    assert tuple(values) == ALGORITHMS, result.stdout
    return values


def brute_force(c, items):
    """Try all subsets (their weight and profit sums are built by doubling); only for small n."""
    weights, profits = [0], [0]
    for w, p in items:
        weights += [x + w for x in weights]
        profits += [x + p for x in profits]
    return max(p for w, p in zip(weights, profits) if w <= c)


def python_dp(c, items):
    """Plain DP over capacities written separately from the C++ code."""
    best = [0] * (c + 1)
    for w, p in items:
        if w <= c:
            best = best[:w] + [max(a, b + p) for a, b in zip(best[w:], best)]
    return best[c]


def expect(binary, c, items, answer, label, refused=()):
    """All algorithms must print the answer, except the ones listed as refusing this input."""
    values = run_all(binary, instance_text(c, items))
    for name in ALGORITHMS:
        want = None if name in refused else answer
        assert values[name] == want, f'{label}: {name} gave {values[name]}, expected {want}'


def hand_made(binary):
    """Small cases with known answers, chosen to hit special paths."""
    book = [(2, 6), (3, 5), (6, 8), (7, 9), (5, 6), (9, 7), (4, 3)]  # example of Fig. 3.7 in [1]
    cases = [
        ('no items', 10, [], 0),
        ('capacity 0', 0, [(1, 5), (2, 7)], 0),
        ('every item too heavy', 3, [(4, 10), (5, 1)], 0),
        ('everything fits', 100, [(3, 4), (5, 6), (7, 1)], 11),
        ('one item fills the knapsack', 7, [(7, 9), (3, 4), (3, 4)], 9),
        ('two light items beat one heavy item', 7, [(7, 9), (3, 5), (4, 5)], 10),
        ('all items equally efficient', 10, [(2, 4), (3, 6), (4, 8), (5, 10)], 20),
        ('zero profits', 5, [(1, 0), (2, 0), (3, 0)], 0),
        ('greedy prefix is not optimal', 10, [(6, 7), (5, 5), (5, 5)], 10),
        ('book example', 9, book, 15),
        ('many copies of one item', 1000, [(7, 3)] * 300, 3 * (1000 // 7)),
    ]
    for label, c, items, answer in cases:
        assert (brute_force(c, items) if len(items) <= 16 else python_dp(c, items)) == answer, label
        expect(binary, c, items, answer, label)
    return len(cases)


def random_small(binary, count, rng):
    """Random instances with n <= 12, compared with brute force."""
    for case in range(count):
        n = rng.randint(1, 12)
        wmax = rng.choice((1, 2, 3, 5, 10, 30, 100))
        pmax = rng.choice((1, 3, 10, 100))
        items = [(rng.randint(1, wmax), rng.randint(0, pmax)) for _ in range(n)]
        c = rng.randint(0, sum(w for w, _ in items) + 2)
        expect(binary, c, items, brute_force(c, items), f'random small {case}')
    return count


def many_small_items(binary, count, rng):
    """Up to 300 items with weights 1..8, so weight classes hold many items; compared with Python DP."""
    for case in range(count):
        n = rng.randint(20, 200)
        wmax = rng.randint(1, 8)
        items = [(rng.randint(1, wmax), rng.randint(0, 50)) for _ in range(n)]
        c = rng.randint(0, sum(w for w, _ in items))
        expect(binary, c, items, python_dp(c, items), f'many small items {case}')
    return count


def generated(binary, gen, quick):
    """Generator instances of all five types; Python DP checks the smaller ones."""
    count = 0
    sizes = (50,) if quick else (50, 100, 200)
    ranges = (10, 100) if quick else (10, 100, 1000)
    for kind in ('uncorrelated', 'weak', 'strong', 'inverse', 'subset'):
        for n in sizes:
            for R in ranges:
                for seed in (1, 2):
                    text = subprocess.run([str(gen), str(n), str(R), kind, str(seed)],
                                          capture_output=True, text=True, check=True).stdout
                    lines = text.split('\n')
                    c = int(lines[0].split()[1])
                    items = [tuple(map(int, line.split())) for line in lines[1:] if line]
                    values = run_all(binary, text)
                    if n * c <= 300_000:
                        answer = python_dp(c, items)
                    else:
                        answer = values['dp1']
                    refused = ('small',) if max(w for w, _ in items) > 2000 else ()
                    for name in ALGORITHMS:
                        want = None if name in refused else answer
                        assert values[name] == want, f'{kind} {n} {R} {seed}: {name} {values[name]} != {want}'
                    count += 1
    # the generator must be repeatable
    first = subprocess.run([str(gen), '30', '50', 'weak', '7'], capture_output=True, text=True).stdout
    again = subprocess.run([str(gen), '30', '50', 'weak', '7'], capture_output=True, text=True).stdout
    assert first == again
    return count


def large_cases(binary, gen):
    """Inputs too big for a full table: large capacity, and many items with small weights."""
    count = 0
    # capacity about 2.5e9: dp1 and small refuse, the list methods must agree
    for kind in ('uncorrelated', 'weak', 'inverse'):
        text = subprocess.run([str(gen), '60', '100000000', kind, '3'],
                              capture_output=True, text=True, check=True).stdout
        values = run_all(binary, text)
        assert values['dp1'] is None and values['small'] is None
        assert values['lists'] == values['lists-sorted'] == values['bounds'], values
        count += 1
    # 20,000 items with weights up to 10: dp1 still fits in memory, all five must agree
    for kind in ('uncorrelated', 'strong', 'subset'):
        text = subprocess.run([str(gen), '20000', '10', kind, '4'],
                              capture_output=True, text=True, check=True).stdout
        values = run_all(binary, text)
        assert len(set(values.values())) == 1 and None not in values.values(), values
        count += 1
    return count


def bad_inputs(binary):
    """The program must reject malformed input instead of guessing."""
    for text in ('2 5\n1 1\n', '1 5\n0 3\n', '1 -1\n1 1\n', '1 5\n2 -4\n', ''):
        result = subprocess.run([str(binary), 'all'], input=text, capture_output=True, text=True)
        assert result.returncode == 1, text
    return 5


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/knapsack')
    parser.add_argument('--gen', type=Path, default=ROOT / 'build/gen')
    parser.add_argument('--quick', action='store_true', help='fewer cases, for the sanitizer build')
    args = parser.parse_args()
    rng = random.Random(2026)
    groups = [
        ('hand-made cases', lambda: hand_made(args.binary)),
        ('random small vs brute force', lambda: random_small(args.binary, 500 if args.quick else 3000, rng)),
        ('many small items vs Python DP', lambda: many_small_items(args.binary, 50 if args.quick else 200, rng)),
        ('generated instances', lambda: generated(args.binary, args.gen, args.quick)),
        ('bad inputs rejected', lambda: bad_inputs(args.binary)),
    ]
    if not args.quick:
        groups.append(('large instances, algorithms agree', lambda: large_cases(args.binary, args.gen)))
    for label, test in groups:
        print(f'PASS {label}: {test()} cases', flush=True)


if __name__ == '__main__':
    main()
