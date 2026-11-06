"""Run the timing experiments with code/gen.cpp and code/bench.cpp, then draw the report figures."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / 'data'
TYPES = ('uncorrelated', 'weak', 'strong', 'inverse', 'subset')
SEEDS = (1, 2, 3)
RUNS = 10            # measured runs after one warmup
SKIP_MS = 500        # stop growing the input for an algorithm once its median passes this
# each experiment: the parameter that grows, its values, the fixed parameters and the algorithms
EXPERIMENTS = {
    'size': {'vary': 'n', 'values': (250, 500, 1000, 2000, 4000), 'fixed': {'R': 1000},
             'types': TYPES, 'algorithms': ('dp1', 'lists', 'lists-sorted', 'bounds')},
    'range': {'vary': 'R', 'values': (100, 1000, 10000, 100000, 1000000), 'fixed': {'n': 1000},
              'types': TYPES, 'algorithms': ('dp1', 'lists', 'bounds')},
    'small-n': {'vary': 'n', 'values': (1000, 10000, 100000, 1000000), 'fixed': {'R': 100},
                'types': TYPES, 'algorithms': ('dp1', 'bounds', 'small')},
    'small-w': {'vary': 'R', 'values': (10, 20, 40, 80, 160, 320), 'fixed': {'n': 10000},
                'types': TYPES, 'algorithms': ('dp1', 'bounds', 'small')},
}
LABELS = {'dp1': 'DP-1', 'lists': 'DP-with-Lists', 'lists-sorted': 'DP-with-Lists, sorted items',
          'bounds': 'DP-with-Upper-Bounds', 'small': 'Small-weight method (bonus)'}
STYLE = {'dp1': ('#2a78d6', 'o'), 'lists': ('#eb6834', 's'), 'lists-sorted': ('#1baf7a', 'v'),
         'bounds': ('#eda100', 'D'), 'small': ('#e87ba4', '^')}


def instance(kind, n, R, seed):
    """Generate one instance file with code/gen.cpp, or reuse it if it exists."""
    path = DATA / 'instances' / f'{kind}-{n}-{R}-{seed}.in'
    if not path.exists():
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open('w') as out:
            subprocess.run([str(ROOT / 'build/gen'), str(n), str(R), kind, str(seed)], stdout=out, check=True)
    return path


def bench(algorithm, path):
    """Time one algorithm on one file: 1 warmup and RUNS measured runs inside code/bench.cpp."""
    result = subprocess.run([str(ROOT / 'build/bench'), algorithm, str(path), str(RUNS)],
                            capture_output=True, text=True, timeout=1800)
    if result.returncode != 0:
        return None  # the algorithm refused this input, e.g. dp1 with a huge capacity
    name, n, c, wmax, value, work, median, low, high = result.stdout.split()
    return {'n': int(n), 'c': int(c), 'wmax': int(wmax), 'value': int(value), 'work': int(work),
            'median_ms': float(median), 'min_ms': float(low), 'max_ms': float(high)}


def run(name):
    """Run one experiment and save every measured instance to data/<name>.json."""
    exp = EXPERIMENTS[name]
    rows = []
    for kind in exp['types']:
        skipped = set()
        for value in exp['values']:
            params = dict(exp['fixed'], **{exp['vary']: value})
            for seed in SEEDS:
                path = instance(kind, params['n'], params['R'], seed)
                answers = {}
                for algorithm in exp['algorithms']:
                    if algorithm in skipped:
                        continue
                    row = bench(algorithm, path)
                    if row is None:
                        continue
                    answers[algorithm] = row['value']
                    rows.append(dict(row, experiment=name, type=kind, R=params['R'], seed=seed,
                                     algorithm=algorithm))
                    print(f"{name} {kind} {exp['vary']}={value} seed {seed} {algorithm}: "
                          f"{row['median_ms']:.3f} ms, work {row['work']}", flush=True)
                assert len(set(answers.values())) <= 1, f'algorithms disagree: {answers}'
            # an algorithm whose median over the seeds is already slow is not run on larger inputs
            for algorithm in exp['algorithms']:
                times = [r['median_ms'] for r in rows if r['type'] == kind and r['algorithm'] == algorithm
                         and r['n'] == params['n'] and r['R'] == params['R']]
                if times and statistics.median(times) > SKIP_MS:
                    skipped.add(algorithm)
    DATA.mkdir(exist_ok=True)
    (DATA / f'{name}.json').write_text(json.dumps(rows, indent=1) + '\n')
    return rows


def summary(rows, exp):
    """Median over seeds of the per-instance medians, with the fastest and slowest seed."""
    groups = {}
    for r in rows:
        key = (r['type'], r['algorithm'], r['n'] if exp['vary'] == 'n' else r['R'])
        groups.setdefault(key, []).append(r)
    result = {}
    for key, group in groups.items():
        if len(group) < len(SEEDS):
            continue  # only complete groups are drawn
        times = [g['median_ms'] for g in group]
        result[key] = {'median': statistics.median(times), 'min': min(times), 'max': max(times),
                       'work': statistics.median(g['work'] for g in group),
                       'c': statistics.median(g['c'] for g in group)}
    return result


def short(value):
    """Write 250, 1k, 10k or 1M for axis labels."""
    for size, suffix in ((1000000, 'M'), (1000, 'k')):
        if value >= size and value % size == 0:
            return f'{value // size}{suffix}'
    return str(value)


def plot(name, rows):
    """One panel per instance type; time on a log scale against the growing parameter."""
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    import matplotlib.ticker
    exp = EXPERIMENTS[name]
    data = summary(rows, exp)
    fig, axes = plt.subplots(2, 3, figsize=(6.6, 4.6), sharey=True)
    for ax, kind in zip(axes.flat, exp['types']):
        for algorithm in exp['algorithms']:
            xs = [v for v in exp['values'] if (kind, algorithm, v) in data]
            if not xs:
                continue
            ys = [data[(kind, algorithm, v)]['median'] for v in xs]
            err = [[y - data[(kind, algorithm, v)]['min'] for v, y in zip(xs, ys)],
                   [data[(kind, algorithm, v)]['max'] - y for v, y in zip(xs, ys)]]
            color, marker = STYLE[algorithm]
            ax.errorbar(xs, ys, yerr=err, color=color, marker=marker, markersize=4.5, linewidth=1.6,
                        capsize=2, label=LABELS[algorithm])
        ax.set_xscale('log')
        ax.set_yscale('log')
        ax.set_xticks(exp['values'])
        ax.set_xticklabels([short(v) for v in exp['values']])
        ax.xaxis.set_minor_locator(matplotlib.ticker.NullLocator())
        ax.set_title(kind, fontsize=9)
        ax.tick_params(labelsize=7)
        ax.grid(True, which='major', color='#e4e4e0', linewidth=0.6)
        for side in ('top', 'right'):
            ax.spines[side].set_visible(False)
    for ax in axes[:, 0]:
        ax.set_ylabel('median time (ms)', fontsize=8)
    for ax in axes[1, :2]:
        ax.set_xlabel('number of items n' if exp['vary'] == 'n' else 'weight range R', fontsize=8)
    axes[0, 2].set_xlabel('number of items n' if exp['vary'] == 'n' else 'weight range R', fontsize=8)
    legend_ax = axes[1, 2]
    legend_ax.axis('off')
    handles, labels = axes[0, 0].get_legend_handles_labels()
    legend_ax.legend(handles, labels, loc='center', frameon=False, fontsize=8)
    fig.tight_layout()
    for suffix in ('png', 'pdf'):
        fig.savefig(ROOT / f'figures/{name}.{suffix}', dpi=200)
    plt.close(fig)


def table(name, rows):
    """Print a Markdown table of median times (ms) and median work for the report."""
    exp = EXPERIMENTS[name]
    data = summary(rows, exp)
    print(f'\n{name}: median ms (work)')
    for kind in exp['types']:
        for algorithm in exp['algorithms']:
            cells = []
            for v in exp['values']:
                d = data.get((kind, algorithm, v))
                cells.append('-' if d is None else f"{d['median']:.3g} ({d['work']:.3g})")
            print(f'| {kind} | {algorithm} | ' + ' | '.join(cells) + ' |')


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
