"""Run the timing experiments with code/gen.cpp and the timing mode of code/main.cpp, then draw the figures."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / 'data'
SEEDS = (1, 2, 3)
RUNS = 10              # measured runs after one warmup
BRUTE_LIMIT = 4000     # brute force needs n * n numbers, so it is only run up to this n
# each experiment: the parameter that grows, its values, the fixed parameter, data types and methods
EXPERIMENTS = {
    'size': {'vary': 'n', 'values': (250, 500, 1000, 2000, 4000, 10000, 30000, 100000),
             'fixed': {'d': 1000000}, 'types': ('random', 'close', 'equal'),
             'methods': ('brute', 'heap', 'search')},
    'money': {'vary': 'd', 'values': (1000, 10000, 100000, 1000000, 10000000),
              'fixed': {'n': 100000}, 'types': ('random', 'close', 'equal'),
              'methods': ('heap', 'search')},
}
LABELS = {'brute': 'Brute force', 'heap': 'Heap method', 'search': 'Threshold method'}
# colors checked with the dataviz palette validator (all pairs pass)
STYLE = {'heap': ('#2a78d6', 'o'), 'search': ('#eb6834', 's'), 'brute': ('#1baf7a', '^')}


def instance(kind, n, d, seed):
    """Generate one input file with code/gen.cpp, or reuse it if it exists."""
    path = DATA / 'instances' / f'{kind}-{n}-{d}-{seed}.in'
    if not path.exists():
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open('w') as out:
            subprocess.run([str(ROOT / 'build/gen'), str(n), str(d), kind, str(seed)], stdout=out, check=True)
    return path


def bench(method, path):
    """Time one method on one file: the program does 1 warmup and RUNS measured runs itself."""
    with path.open() as source:
        result = subprocess.run([str(ROOT / 'build/coupons'), method, str(RUNS)], stdin=source,
                                capture_output=True, text=True, timeout=1800, check=True)
    name, n, d, items, left, median, low, high = result.stdout.split()
    return {'n': int(n), 'd': int(d), 'items': int(items), 'left': int(left),
            'median_ms': float(median), 'min_ms': float(low), 'max_ms': float(high)}


def run(name):
    """Run one experiment and save every measured input to data/<name>.json."""
    exp = EXPERIMENTS[name]
    rows = []
    for kind in exp['types']:
        for value in exp['values']:
            params = dict(exp['fixed'], **{exp['vary']: value})
            for seed in SEEDS:
                path = instance(kind, params['n'], params['d'], seed)
                answers = set()
                for method in exp['methods']:
                    if method == 'brute' and params['n'] > BRUTE_LIMIT:
                        continue
                    row = bench(method, path)
                    answers.add((row['items'], row['left']))
                    rows.append(dict(row, experiment=name, type=kind, seed=seed, method=method))
                    print(f"{name} {kind} {exp['vary']}={value} seed {seed} {method}: "
                          f"{row['median_ms']:.3f} ms, items {row['items']}", flush=True)
                assert len(answers) == 1, f'methods disagree: {answers}'
    DATA.mkdir(exist_ok=True)
    (DATA / f'{name}.json').write_text(json.dumps(rows, indent=1) + '\n')
    return rows


def summary(rows, exp):
    """Median over the seeds of the per-input medians, with the fastest and slowest input."""
    groups = {}
    for r in rows:
        groups.setdefault((r['type'], r['method'], r[exp['vary']]), []).append(r)
    result = {}
    for key, group in groups.items():
        times = [g['median_ms'] for g in group]
        result[key] = {'median': statistics.median(times), 'min': min(times), 'max': max(times),
                       'items': statistics.median(g['items'] for g in group)}
    return result


def short(value):
    """Write 250, 1k, 10k or 1M for axis labels."""
    for size, suffix in ((1000000, 'M'), (1000, 'k')):
        if value >= size and value % size == 0:
            return f'{value // size}{suffix}'
    return str(value)


def plot(name, rows):
    """One panel per data type; time on a log scale against the growing parameter."""
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    import matplotlib.ticker
    exp = EXPERIMENTS[name]
    data = summary(rows, exp)
    fig, axes = plt.subplots(1, len(exp['types']), figsize=(7.2, 2.9), sharey=True)
    for ax, kind in zip(axes, exp['types']):
        for method in exp['methods']:
            xs = [v for v in exp['values'] if (kind, method, v) in data]
            ys = [data[(kind, method, v)]['median'] for v in xs]
            err = [[y - data[(kind, method, v)]['min'] for v, y in zip(xs, ys)],
                   [data[(kind, method, v)]['max'] - y for v, y in zip(xs, ys)]]
            color, marker = STYLE[method]
            ax.errorbar(xs, ys, yerr=err, color=color, marker=marker, markersize=4.5, linewidth=1.6,
                        capsize=2, label=LABELS[method])
        ax.set_xscale('log')
        ax.set_yscale('log')
        ticks = exp['values'][::2] if len(exp['values']) > 6 else exp['values']
        ax.set_xticks(ticks)
        ax.set_xticklabels([short(v) for v in ticks])
        ax.xaxis.set_minor_locator(matplotlib.ticker.NullLocator())
        ax.set_title(kind, fontsize=9)
        ax.set_xlabel('number of items N' if exp['vary'] == 'n' else 'money D', fontsize=8)
        ax.tick_params(labelsize=7)
        ax.grid(True, which='major', color='#e4e4e0', linewidth=0.6)
        for side in ('top', 'right'):
            ax.spines[side].set_visible(False)
    axes[0].set_ylabel('median time (ms)', fontsize=8)
    # the lower right corner is empty in the size figure, the upper left one in the money figure
    axes[0].legend(frameon=False, fontsize=7.5, loc='lower right' if name == 'size' else 'upper left')
    fig.tight_layout()
    for suffix in ('png', 'pdf'):
        fig.savefig(ROOT / f'figures/{name}.{suffix}', dpi=200)
    plt.close(fig)


def table(name, rows):
    """Print the median times (ms) and the number of items bought, for the report tables."""
    exp = EXPERIMENTS[name]
    data = summary(rows, exp)
    print(f'\n{name}: median ms (items bought)')
    print('| type | method | ' + ' | '.join(short(v) for v in exp['values']) + ' |')
    for kind in exp['types']:
        for method in exp['methods']:
            cells = []
            for v in exp['values']:
                d = data.get((kind, method, v))
                cells.append('-' if d is None else f"{d['median']:.3g} ({d['items']:.0f})")
            print(f'| {kind} | {method} | ' + ' | '.join(cells) + ' |')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('experiments', nargs='*', default=list(EXPERIMENTS))
    parser.add_argument('--plot-only', action='store_true', help='redraw from data/*.json')
    args = parser.parse_args()
    for name in args.experiments:
        rows = json.loads((DATA / f'{name}.json').read_text()) if args.plot_only else run(name)
        plot(name, rows)
        table(name, rows)


if __name__ == '__main__':
    main()
