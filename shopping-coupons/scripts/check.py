"""Correctness tests: compare the three methods with hand-made answers and two Python oracles."""
import argparse
from itertools import combinations
from pathlib import Path
import random
import subprocess

ROOT = Path(__file__).resolve().parents[1]
METHODS = ('heap', 'search', 'brute')


def input_text(d, prices, coupons):
    """Write a case in the input format of the problem."""
    return f'{len(prices)} {d}\n' + ' '.join(map(str, prices)) + '\n' + ' '.join(map(str, coupons)) + '\n'


def run(binary, method, text):
    """Run one method and return its exact output."""
    result = subprocess.run([str(binary), method], input=text, capture_output=True, text=True, timeout=120)
    assert result.returncode == 0 and not result.stderr, (method, result.stderr)
    return result.stdout


def expect(binary, text, answer, label, methods=METHODS):
    """Every method must print exactly "items left" and a newline."""
    want = f'{answer[0]} {answer[1]}\n'
    for method in methods:
        got = run(binary, method, text)
        assert got == want, f'{label}: {method} printed {got!r}, expected {want!r}'


def sorted_oracle(d, prices, coupons):
    """Sort all pair costs in Python and buy from the cheapest."""
    items, left = 0, d
    for cost in sorted(p - c for p in prices for c in coupons):
        if cost > left:
            break
        items, left = items + 1, left - cost
    return items, left


def subset_oracle(d, prices, coupons):
    """Try every subset of pairs: most items first, then most money left. Does not use the greedy idea."""
    costs = [p - c for p in prices for c in coupons]
    best = (0, d)
    for size in range(1, len(costs) + 1):
        for chosen in combinations(costs, size):
            total = sum(chosen)
            if total <= d:
                best = max(best, (size, d - total))
    return best


def hand_made(binary):
    """Small cases whose answers were worked out by hand."""
    big = 10 ** 9
    cases = [
        ('sample from the problem', 30, [12, 20, 15, 10], [9, 6, 8, 7], (8, 2)),
        ('one pair, too expensive', 5, [10], [4], (0, 5)),
        ('one pair, exact money', 6, [10], [4], (1, 0)),
        ('one pair, each pair only once', 100, [10], [4], (1, 94)),
        ('buy every pair, nothing left', 18, [5, 7], [1, 2], (4, 0)),
        ('one dollar short of every pair', 17, [5, 7], [1, 2], (3, 5)),
        ('more money than all pairs cost', 1000, [7, 5], [2, 1], (4, 982)),
        ('all costs equal', 5, [4, 4, 4], [3, 3, 3], (5, 0)),
        ('one dollar, cheapest cost 1', 1, [3, 9], [2, 1], (1, 0)),
        ('one dollar, cheapest cost 2', 1, [3, 9], [1, 1], (0, 1)),
        ('values near 10^9', 10 ** 6, [big, big], [big - 1, 1], (2, 999998)),
        ('same cost from different pairs', 8, [5, 6], [3, 2], (3, 0)),
    ]
    for label, d, prices, coupons, answer in cases:
        assert sorted_oracle(d, prices, coupons) == answer, label
        assert subset_oracle(d, prices, coupons) == answer, label
        expect(binary, input_text(d, prices, coupons), answer, label)
    return len(cases)


def random_case(rng, n, spread):
    """Coupons in 1..spread and prices just above them, so the costs are small and often equal."""
    coupons = [rng.randint(1, spread) for _ in range(n)]
    prices = [rng.randint(spread + 1, 2 * spread + 1) for _ in range(n)]
    return prices, coupons


def tiny_exhaustive(binary, count, rng):
    """n <= 3: compare with all subsets of pairs, which checks the greedy idea itself."""
    for case in range(count):
        n = rng.randint(1, 3)
        prices, coupons = random_case(rng, n, rng.choice((1, 2, 5, 20)))
        total = sum(p - c for p in prices for c in coupons)
        d = rng.randint(1, total + 3)
        answer = subset_oracle(d, prices, coupons)
        assert answer == sorted_oracle(d, prices, coupons), (d, prices, coupons)
        expect(binary, input_text(d, prices, coupons), answer, f'tiny {case}')
    return count


def random_small(binary, count, rng):
    """n <= 40 with small or large values, compared with the Python sort."""
    for case in range(count):
        n = rng.randint(1, 40)
        prices, coupons = random_case(rng, n, rng.choice((1, 3, 10, 1000, 4 * 10 ** 8)))
        total = sum(p - c for p in prices for c in coupons)
        d = rng.choice((1, rng.randint(1, 10 ** 6), min(10 ** 6, rng.randint(1, total + 5))))
        expect(binary, input_text(d, prices, coupons), sorted_oracle(d, prices, coupons), f'random {case}')
    return count


def parse(text):
    """Read back a generated case."""
    lines = text.split('\n')
    d = int(lines[0].split()[1])
    return d, list(map(int, lines[1].split())), list(map(int, lines[2].split()))


def generated(binary, gen, quick):
    """All four generator types at several sizes, compared with the Python sort."""
    count = 0
    sizes = (1, 2, 50) if quick else (1, 2, 50, 300)
    for kind in ('random', 'close', 'equal', 'far'):
        for n in sizes:
            for d in (1, 1000, 10 ** 6):
                for seed in (1, 2):
                    text = subprocess.run([str(gen), str(n), str(d), kind, str(seed)],
                                          capture_output=True, text=True, check=True).stdout
                    d_read, prices, coupons = parse(text)
                    assert d_read == d and len(prices) == len(coupons) == n
                    assert max(coupons) < min(prices), 'generator broke the promise of the problem'
                    expect(binary, text, sorted_oracle(d, prices, coupons), f'{kind} {n} {d} {seed}')
                    count += 1
    return count


def large_cases(binary, gen):
    """Largest sizes: the methods must agree with each other."""
    count = 0
    # n = 4000 is still small enough for brute force
    for kind in ('random', 'close', 'equal'):
        text = subprocess.run([str(gen), '4000', '1000000', kind, '5'],
                              capture_output=True, text=True, check=True).stdout
        outputs = {run(binary, method, text) for method in METHODS}
        assert len(outputs) == 1, (kind, outputs)
        count += 1
    # n = 10^5 and d = 10^6, the limits of the problem; "equal" buys 10^6 items
    for kind in ('random', 'close', 'equal', 'far'):
        for seed in (6, 7):
            text = subprocess.run([str(gen), '100000', '1000000', kind, str(seed)],
                                  capture_output=True, text=True, check=True).stdout
            outputs = {run(binary, method, text) for method in ('heap', 'search')}
            assert len(outputs) == 1, (kind, seed, outputs)
            if kind == 'equal':
                assert outputs == {'1000000 0\n'}
            if kind == 'far':
                assert outputs == {'0 1000000\n'}
            count += 1
    return count


MASK = (1 << 64) - 1


class MT64:
    """The mt19937_64 engine, written from the C++ standard, to check that gen.cpp is portable."""

    def __init__(self, seed):
        self.mt = [seed & MASK] + [0] * 311
        for i in range(1, 312):
            self.mt[i] = (6364136223846793005 * (self.mt[i - 1] ^ (self.mt[i - 1] >> 62)) + i) & MASK
        self.i = 312

    def __call__(self):
        if self.i >= 312:
            for k in range(312):
                y = (self.mt[k] & 0xFFFFFFFF80000000) | (self.mt[(k + 1) % 312] & 0x7FFFFFFF)
                self.mt[k] = self.mt[(k + 156) % 312] ^ (y >> 1) ^ (0xB5026F5AA96619E9 if y & 1 else 0)
            self.i = 0
        y = self.mt[self.i]
        self.i += 1
        y ^= (y >> 29) & 0x5555555555555555
        y ^= (y << 17) & 0x71D67FFFEDA60000
        y ^= (y << 37) & 0xFFF7EEE000000000
        return (y ^ (y >> 43)) & MASK


RANGES = {  # type: (price_lo, price_hi, coupon_lo, coupon_hi), the same numbers as in code/gen.cpp
    'random': (1000001, 2000000, 1, 1000000),
    'close': (500000000, 500000000 + 1999, 500000000 - 2000, 500000000 - 1),
    'equal': (999999999, 999999999, 999999998, 999999998),
    'far': (500000000, 1000000000, 1, 100000000),
}


def python_gen(n, d, kind, seed):
    """The same rules as code/gen.cpp, written again in Python."""
    rng = MT64(seed)

    def uniform(lo, hi):
        size = hi - lo + 1
        limit = MASK - MASK % size
        x = rng()
        while x >= limit:
            x = rng()
        return lo + x % size

    price_lo, price_hi, coupon_lo, coupon_hi = RANGES[kind]
    prices = [uniform(price_lo, price_hi) for _ in range(n)]
    coupons = [uniform(coupon_lo, coupon_hi) for _ in range(n)]
    return input_text(d, prices, coupons)


def portable_generator(gen):
    """gen.cpp must print exactly what the Python version of the standard engine gives."""
    count = 0
    for kind in RANGES:
        for n, d, seed in ((1, 1, 0), (20, 7, 1), (500, 1000, 2), (300, 1000000, 3)):
            text = subprocess.run([str(gen), str(n), str(d), kind, str(seed)],
                                  capture_output=True, text=True, check=True).stdout
            assert text == python_gen(n, d, kind, seed), f'generator differs: {kind} {n} {d} {seed}'
            count += 1
    return count


def bad_inputs(binary):
    """Broken input must give an error instead of a wrong answer."""
    cases = ('', '0 5\n\n\n', '2 5\n3 4\n1\n', '2 x\n3 4\n1 1\n')
    for text in cases:
        result = subprocess.run([str(binary)], input=text, capture_output=True, text=True)
        assert result.returncode == 1 and result.stdout == '', text
    return len(cases)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/coupons')
    parser.add_argument('--gen', type=Path, default=ROOT / 'build/gen')
    parser.add_argument('--quick', action='store_true', help='fewer cases, for the sanitizer build')
    args = parser.parse_args()
    rng = random.Random(2026)
    groups = [
        ('hand-made cases', lambda: hand_made(args.binary)),
        ('tiny cases vs all subsets', lambda: tiny_exhaustive(args.binary, 100 if args.quick else 500, rng)),
        ('random small vs Python sort', lambda: random_small(args.binary, 150 if args.quick else 1000, rng)),
        ('generated cases', lambda: generated(args.binary, args.gen, args.quick)),
        ('bad inputs rejected', lambda: bad_inputs(args.binary)),
        ('generator matches the C++ standard engine', lambda: portable_generator(args.gen)),
    ]
    if not args.quick:
        groups.append(('large cases, methods agree', lambda: large_cases(args.binary, args.gen)))
    total = 0
    for label, test in groups:
        count = test()
        total += count
        print(f'PASS {label}: {count} cases', flush=True)
    print(f'PASS all: {total} cases')


if __name__ == '__main__':
    main()
