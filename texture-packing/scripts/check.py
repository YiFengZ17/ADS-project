"""Correctness tests: every packing must be legal, match a Python version of the same rule, and keep the known bounds."""
import argparse
import bisect
import heapq
from pathlib import Path
import random
import subprocess

ROOT = Path(__file__).resolve().parents[1]
ALGORITHMS = ('ffdh', 'ffdh-scan', 'nfdh', 'bfdh', 'sleator')
MASK = (1 << 64) - 1


def input_text(W, rects):
    """Write a case in the input format: "n W", then one "w h" per line."""
    return f'{len(rects)} {W}\n' + ''.join(f'{w} {h}\n' for w, h in rects)


def run(binary, algo, text):
    """Run one algorithm and return (height, lower bound, list of corners)."""
    result = subprocess.run([str(binary), algo], input=text, capture_output=True, text=True, timeout=600)
    assert result.returncode == 0 and not result.stderr, (algo, result.stderr)
    lines = result.stdout.split('\n')
    height, lb = map(int, lines[0].split())
    pos = [tuple(map(int, line.split())) for line in lines[1:] if line]
    return height, lb, pos


def overlaps(W, rects, pos):
    """Return True if some texture leaves the strip or two textures overlap (a sweep over x)."""
    events = []
    for i, ((w, h), (x, y)) in enumerate(zip(rects, pos)):
        if x < 0 or y < 0 or x + w > W:
            return True
        events.append((x + w, 0, i))  # 0: the texture ends here; ends go before starts at the same x
        events.append((x, 1, i))
    events.sort()
    active = []  # (bottom, top, id) of the textures that cross the sweep line, sorted by bottom
    for _, kind, i in events:
        y0, y1 = pos[i][1], pos[i][1] + rects[i][1]
        if kind == 0:
            del active[bisect.bisect_left(active, (y0, y1, i))]  # binary search, the keys are unique
            continue
        k = bisect.bisect_left(active, (y0, y1, i))
        # active intervals are disjoint, so only the neighbors in y order can overlap the new one
        if k > 0 and active[k - 1][1] > y0:
            return True
        if k < len(active) and active[k][0] < y1:
            return True
        active.insert(k, (y0, y1, i))
    return False


def lower_bound(W, rects):
    """Area bound, tallest texture, and the stack of textures wider than W/2."""
    area = sum(w * h for w, h in rects)
    return max(-(-area // W), max(h for _, h in rects), sum(h for w, h in rects if 2 * w > W))


def order(rects):
    """Tall to short; equal heights go wider first, then by input order."""
    return sorted(range(len(rects)), key=lambda i: (-rects[i][1], -rects[i][0], i))


def py_levels(W, rects, choose):
    """Level packing in Python; choose(W, used widths, w) returns the level to use, or None."""
    pos = [None] * len(rects)
    base, used, top = [], [], 0
    for i in order(rects):
        w, h = rects[i]
        k = choose(W, used, w)
        if k is None:
            base.append(top)
            used.append(0)
            top += h
            k = len(base) - 1
        pos[i] = (used[k], base[k])
        used[k] += w
    return top, pos


def first_fit(W, used, w):
    """The lowest level with enough room, or None."""
    return next((k for k, u in enumerate(used) if W - u >= w), None)


def best_fit(W, used, w):
    """The level with the least free width that is still enough (lowest on ties), or None."""
    fits = [(W - u, k) for k, u in enumerate(used) if W - u >= w]
    return min(fits)[1] if fits else None


def next_fit(W, used, w):
    """The newest level if the texture fits there, or None."""
    return len(used) - 1 if used and W - used[-1] >= w else None


def py_sleator(W, rects):
    """Sleator's algorithm written again in Python."""
    pos = [None] * len(rects)
    top, narrow = 0, []
    for i in order(rects):
        if 2 * rects[i][0] > W:
            pos[i] = (0, top)
            top += rects[i][1]
        else:
            narrow.append(i)
    half = W // 2
    tops = [top, top]  # left half, right half
    x, k = 0, 0
    while k < len(narrow) and x + rects[narrow[k]][0] <= W:
        w, h = rects[narrow[k]]
        pos[narrow[k]] = (x, top)
        if x < half:
            tops[0] = max(tops[0], top + h)
        if x + w > half:
            tops[1] = max(tops[1], top + h)
        x, k = x + w, k + 1
    while k < len(narrow):
        side = 0 if tops[0] <= tops[1] else 1
        x0, width, base = (0, half, tops[0]) if side == 0 else (half, W - half, tops[1])
        level_h, x = rects[narrow[k]][1], 0
        while k < len(narrow) and x + rects[narrow[k]][0] <= width:
            pos[narrow[k]] = (x0 + x, base)
            x, k = x + rects[narrow[k]][0], k + 1
        tops[side] = base + level_h
    return max(tops), pos


PYTHON = {
    'ffdh': lambda W, r: py_levels(W, r, first_fit),
    'ffdh-scan': lambda W, r: py_levels(W, r, first_fit),
    'nfdh': lambda W, r: py_levels(W, r, next_fit),
    'bfdh': lambda W, r: py_levels(W, r, best_fit),
    'sleator': py_sleator,
}


def check_case(binary, W, rects, label, opt=None, compare=True):
    """Run all algorithms: legal packing, right height, same as Python, and the proven bounds."""
    text = input_text(W, rects)
    area_bound = sum(w * h for w, h in rects) / W
    hmax = max(h for _, h in rects)
    results = {}
    for algo in ALGORITHMS:
        height, lb, pos = run(binary, algo, text)
        assert len(pos) == len(rects), f'{label}: {algo} printed {len(pos)} corners'
        assert not overlaps(W, rects, pos), f'{label}: {algo} has an overlap or leaves the strip'
        assert height == max(y + h for (_, h), (_, y) in zip(rects, pos)), f'{label}: {algo} wrong height'
        assert lb == lower_bound(W, rects) and height >= lb, f'{label}: {algo} lower bound'
        if compare:
            assert (height, pos) == PYTHON[algo](W, rects), f'{label}: {algo} differs from Python'
        if algo != 'sleator':  # level algorithms: H <= 2 * area / W + hmax (derived in the report)
            assert height <= 2 * area_bound + hmax, f'{label}: {algo} breaks the area bound'
        if opt is not None:  # the best height is known, so check the bounds from the papers
            if algo in ('ffdh', 'ffdh-scan'):
                assert height <= 1.7 * opt + hmax, f'{label}: FFDH bound'
            if algo == 'nfdh':
                assert height <= 2 * opt + hmax, f'{label}: NFDH bound'
            if algo == 'sleator':
                assert height <= 2 * opt + hmax / 2, f'{label}: Sleator bound'
        results[algo] = (height, pos)
    assert results['ffdh'] == results['ffdh-scan'], f'{label}: the two FFDH versions differ'
    return results


def hand_made(binary):
    """Small cases with heights worked out by hand: (W, rects, expected heights for ffdh nfdh bfdh sleator)."""
    cases = [
        (10, [(4, 3), (6, 2), (5, 2), (3, 1), (10, 1)], (6, 7, 6, 6)),   # tests/sample.in
        (1, [(1, 5)], (5, 5, 5, 5)),                                    # one texture
        (10, [(10, 2), (10, 3), (10, 1)], (6, 6, 6, 6)),                # every texture fills the width
        (10, [(2, 4)] * 5, (4, 4, 4, 4)),                               # exactly one full level
        (10, [(2, 4)] * 6, (8, 8, 8, 8)),                               # one texture too many
        (10, [(6, 5), (6, 5), (4, 1), (4, 1)], (10, 11, 10, 11)),       # two wide ones; first fit fills the gaps
        (10, [(5, 3), (6, 2), (4, 2), (5, 1)], (6, 6, 5, 5)),           # first fit and best fit differ
        (10, [(5, 4), (5, 4), (5, 4), (5, 1), (5, 1)], (9, 9, 9, 8)),   # exactly W/2 counts as narrow
        (9, [(3, 2), (3, 2), (3, 2), (4, 1), (4, 1), (4, 1)], (4, 4, 4, 4)),  # odd W, two unequal halves
        (10, [(6, 5), (5, 3), (4, 1), (4, 1)], (8, 9, 8, 8)),           # a short one goes back to a lower level
    ]
    for W, rects, want in cases:
        results = check_case(binary, W, rects, f'hand {W} {rects}')
        got = tuple(results[a][0] for a in ('ffdh', 'nfdh', 'bfdh', 'sleator'))
        assert got == want, f'hand {W} {rects}: got {got}, expected {want}'
    return len(cases)


def random_small(binary, count, rng):
    """Random cases with n <= 40, many equal sizes, compared with the Python versions."""
    for t in range(count):
        W = rng.choice((1, 2, 3, 7, 10, 11, 100, 10 ** 9))
        n = rng.randint(1, 40)
        hmax = rng.choice((1, 3, 10, 1000, 10 ** 9))
        rects = [(rng.randint(1, W), rng.randint(1, hmax)) for _ in range(n)]
        check_case(binary, W, rects, f'random {t}')
    return count


def parse(text):
    """Read W and the (w, h) list from an input file's text."""
    lines = text.split('\n')
    W = int(lines[0].split()[1])
    return W, [tuple(map(int, line.split())) for line in lines[1:] if line]


def generated(binary, gen, quick):
    """All generator types: legal, same as Python, bounds hold; cut inputs have best height 10n."""
    sizes = (1, 10, 100, 500) if quick else (1, 2, 10, 50, 100, 300, 1000, 2000)
    shapes = (('1000', '1000'), ('100', '1000'), ('1000', '10')) if not quick else (('1000', '1000'),)
    count = 0
    for kind in ('uniform', 'equalh', 'heavy', 'cut'):
        for n in sizes:
            for seed in (1, 2):
                for wmax, hmax in (shapes if kind != 'cut' else (('1000', '1000'),)):
                    text = subprocess.run([str(gen), str(n), kind, str(seed), wmax, hmax],
                                          capture_output=True, text=True, check=True).stdout
                    W, rects = parse(text)
                    opt = 10 * n if kind == 'cut' else None
                    if opt is not None:
                        assert sum(w * h for w, h in rects) == W * opt, 'cut pieces must fill the rectangle'
                    check_case(binary, W, rects, f'gen {kind} {n} {seed} {wmax} {hmax}', opt=opt)
                    count += 1
    return count


def large_cases(binary, gen):
    """n = 10,000 and 100,000: legal packings and the bounds; the Python versions are too slow here."""
    count = 0
    for n in (10000, 100000):
        for kind in ('uniform', 'equalh', 'heavy', 'cut'):
            text = subprocess.run([str(gen), str(n), kind, '7'], capture_output=True, text=True, check=True).stdout
            W, rects = parse(text)
            check_case(binary, W, rects, f'large {kind} {n}', opt=10 * n if kind == 'cut' else None,
                       compare=n <= 10000 and kind != 'equalh')
            count += 1
    return count


class MT64:
    """The mt19937_64 engine, written from the C++ standard, to check that gen.cpp is portable."""

    def __init__(self, seed):
        """Seed the state the same way as the C++ engine."""
        self.mt = [seed & MASK] + [0] * 311
        for i in range(1, 312):
            self.mt[i] = (6364136223846793005 * (self.mt[i - 1] ^ (self.mt[i - 1] >> 62)) + i) & MASK
        self.i = 312

    def __call__(self):
        """Return the next 64-bit number."""
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


def python_gen(n, kind, seed, wmax=1000, hmax=1000):
    """The same rules as code/gen.cpp, written again in Python."""
    rng = MT64(seed)

    def uniform(lo, hi):
        """Same as uniform() in gen.cpp."""
        size = hi - lo + 1
        limit = MASK - MASK % size
        x = rng()
        while x >= limit:
            x = rng()
        return lo + x % size

    W = 1000
    if kind == 'cut':
        heap = [(-W * 10 * n, 0, W, 10 * n)]  # a max heap on (area, id) as negated pairs
        next_id = 1
        while len(heap) < n:
            _, _, w, h = heapq.heappop(heap)
            if w >= h:
                a = uniform(1, w - 1)
                parts = ((a, h), (w - a, h))
            else:
                a = uniform(1, h - 1)
                parts = ((w, a), (w, h - a))
            for pw, ph in parts:
                heapq.heappush(heap, (-pw * ph, -next_id, pw, ph))
                next_id += 1
        rects = [(w, h) for _, _, w, h in sorted(heap)]
        for i in range(len(rects) - 1, 0, -1):
            j = uniform(0, i)
            rects[i], rects[j] = rects[j], rects[i]
    else:
        rects = []
        for _ in range(n):
            w = uniform(1, wmax)
            h = hmax
            if kind == 'uniform':
                h = uniform(1, hmax)
            if kind == 'heavy':
                h = max(1, hmax // uniform(1, 100))
            rects.append((w, h))
    return input_text(W, rects)


def portable_generator(gen):
    """gen.cpp must print exactly what the Python version of the standard engine gives."""
    count = 0
    for kind in ('uniform', 'equalh', 'heavy', 'cut'):
        for n, seed, wmax, hmax in ((1, 0, 1000, 1000), (20, 1, 1000, 1000), (500, 2, 100, 50), (3000, 3, 700, 1000000)):
            text = subprocess.run([str(gen), str(n), kind, str(seed), str(wmax), str(hmax)],
                                  capture_output=True, text=True, check=True).stdout
            want = python_gen(n, kind, seed, wmax, hmax) if kind != 'cut' else python_gen(n, kind, seed)
            assert text == want, f'generator differs: {kind} {n} {seed}'
            count += 1
    return count


def bad_inputs(binary):
    """Broken input must give an error instead of a wrong answer."""
    cases = ('', '0 5\n', '2 5\n3 4\n', '1 5\n6 1\n', '1 5\n0 1\n', '1 5\n3 0\n', '1 x\n1 1\n', '1 5\n1 1\n7\n')
    for text in cases:
        result = subprocess.run([str(binary)], input=text, capture_output=True, text=True)
        assert result.returncode == 1 and result.stdout == '', text
    result = subprocess.run([str(binary), 'unknown'], input='1 5\n1 1\n', capture_output=True, text=True)
    assert result.returncode == 1 and result.stdout == ''
    return len(cases) + 1


def main():
    """Run all test groups and print one line per group."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/pack')
    parser.add_argument('--gen', type=Path, default=ROOT / 'build/gen')
    parser.add_argument('--quick', action='store_true', help='fewer cases, for the sanitizer build')
    args = parser.parse_args()
    rng = random.Random(2026)
    groups = [
        ('hand-made cases', lambda: hand_made(args.binary)),
        ('random small cases vs Python', lambda: random_small(args.binary, 150 if args.quick else 1000, rng)),
        ('generated cases', lambda: generated(args.binary, args.gen, args.quick)),
        ('bad inputs rejected', lambda: bad_inputs(args.binary)),
        ('generator matches the C++ standard engine', lambda: portable_generator(args.gen)),
    ]
    if not args.quick:
        groups.append(('large cases', lambda: large_cases(args.binary, args.gen)))
    total = 0
    for label, test in groups:
        count = test()
        total += count
        print(f'PASS {label}: {count} cases', flush=True)
    print(f'PASS all: {total} cases')


if __name__ == '__main__':
    main()
