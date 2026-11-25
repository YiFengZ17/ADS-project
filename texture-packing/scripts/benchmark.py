"""Run the experiments with code/gen.cpp and the timing mode of code/main.cpp, then draw the figures."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
DATA = ROOT / 'data'
ALGORITHMS = ('ffdh', 'ffdh-scan', 'nfdh', 'bfdh', 'sleator')
TYPES = ('uniform', 'equalh', 'heavy', 'cut')
SIZES = (10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 30000, 100000)
SCAN_LIMIT = 30000   # the plain-scan FFDH needs n times (number of levels) steps, so it stops here
WIDTHS = (50, 100, 200, 300, 500, 700, 1000)  # wmax values for the width experiment
LABELS = {'ffdh': 'FFDH (ours)', 'ffdh-scan': 'FFDH, plain scan', 'nfdh': 'NFDH', 'bfdh': 'BFDH',
          'sleator': 'Sleator'}
# colors checked with the dataviz palette validator (all adjacent pairs pass); markers add a second cue
STYLE = {'ffdh': ('#2a78d6', 'o'), 'ffdh-scan': ('#6250d6', 'v'), 'nfdh': ('#eb6834', 's'),
         'bfdh': ('#1baf7a', '^'), 'sleator': ('#eda100', 'D')}


def instance(n, kind, seed, wmax=1000, hmax=1000):
    """Generate one input file with code/gen.cpp, or reuse it if it exists."""
    path = DATA / 'instances' / f'{kind}-{n}-{seed}-{wmax}-{hmax}.in'
    if not path.exists():
        path.parent.mkdir(parents=True, exist_ok=True)
        with path.open('w') as out:
            subprocess.run([str(ROOT / 'build/gen'), str(n), kind, str(seed), str(wmax), str(hmax)],
                           stdout=out, check=True)
    return path


def bench(algo, path, runs):
    """Time one algorithm on one file: the program does 1 warmup and `runs` measured runs itself."""
    with path.open() as source:
        result = subprocess.run([str(ROOT / 'build/pack'), algo, str(runs)], stdin=source,
                                capture_output=True, text=True, timeout=1800, check=True)
    name, n, W, height, lb, median, low, high = result.stdout.split()
    return {'n': int(n), 'height': int(height), 'lb': int(lb), 'ratio': int(height) / int(lb),
            'median_ms': float(median), 'min_ms': float(low), 'max_ms': float(high)}


def run_size():
    """Experiment 1: every type and size, 3 seeds, 10 measured runs: time and height / lower bound."""
    rows = []
    for kind in TYPES:
        for n in SIZES:
            for seed in (1, 2, 3):
                path = instance(n, kind, seed)
                for algo in ALGORITHMS:
                    if algo == 'ffdh-scan' and n > SCAN_LIMIT:
                        continue
                    row = dict(bench(algo, path, 10), type=kind, seed=seed, algo=algo)
                    rows.append(row)
                    print(f"size {kind} n={n} seed {seed} {algo}: {row['median_ms']:.4f} ms, "
                          f"ratio {row['ratio']:.4f}", flush=True)
    return rows


def run_width():
    """Experiment 2: n = 1000 uniform data with different largest widths, 10 seeds, quality only."""
    rows = []
    for wmax in WIDTHS:
        for seed in range(1, 11):
            path = instance(1000, 'uniform', seed, wmax, 1000)
            for algo in ('ffdh', 'nfdh', 'bfdh', 'sleator'):
                rows.append(dict(bench(algo, path, 1), wmax=wmax, seed=seed, algo=algo))
        print(f'width wmax={wmax} done', flush=True)
    return rows


def summary(rows, key):
    """Group by (type, algorithm, key) and take the median over the seeds, with min and max."""
    groups = {}
    for r in rows:
        groups.setdefault((r.get('type', 'uniform'), r['algo'], r[key]), []).append(r)
    result = {}
    for k, group in groups.items():
        times = [g['median_ms'] for g in group]
        ratios = [g['ratio'] for g in group]
        result[k] = {'time': statistics.median(times), 'tmin': min(times), 'tmax': max(times),
                     'ratio': statistics.mean(ratios), 'rmin': min(ratios), 'rmax': max(ratios)}
    return result


def setup_axis(ax, title):
    """Common look of one panel: title, small ticks, light grid, no top/right lines."""
    ax.set_title(title, fontsize=9)
    ax.tick_params(labelsize=7)
    ax.grid(True, which='major', color='#e4e4e0', linewidth=0.6)
    for side in ('top', 'right'):
        ax.spines[side].set_visible(False)


def short(value):
    """Write 10, 1k or 100k for axis labels."""
    return f'{value // 1000}k' if value >= 1000 and value % 1000 == 0 else str(value)


def plot_size(rows, plt):
    """Two figures: time against n and height / lower bound against n, one panel per data type."""
    data = summary(rows, 'n')
    for what, ylabel, name in (('time', 'median time (ms)', 'time'), ('ratio', 'height / lower bound', 'ratio')):
        fig, axes = plt.subplots(1, 4, figsize=(7.4, 2.8), sharey=(what == 'time'))
        for ax, kind in zip(axes, TYPES):
            algos = ALGORITHMS if what == 'time' else ('ffdh', 'nfdh', 'bfdh', 'sleator')
            for algo in algos:
                xs = [n for n in SIZES if (kind, algo, n) in data]
                d = [data[(kind, algo, n)] for n in xs]
                ys = [v[what] for v in d]
                low = 'tmin' if what == 'time' else 'rmin'
                high = 'tmax' if what == 'time' else 'rmax'
                err = [[y - v[low] for y, v in zip(ys, d)], [v[high] - y for y, v in zip(ys, d)]]
                color, marker = STYLE[algo]
                ax.errorbar(xs, ys, yerr=err, color=color, marker=marker, markersize=3.5, linewidth=1.4,
                            capsize=1.5, label=LABELS[algo])
            ax.set_xscale('log')
            if what == 'time':
                ax.set_yscale('log')
            ax.set_xticks((10, 100, 1000, 10000, 100000))
            ax.set_xticklabels([short(v) for v in (10, 100, 1000, 10000, 100000)])
            ax.set_xlabel('number of textures n', fontsize=8)
            setup_axis(ax, kind)
        axes[0].set_ylabel(ylabel, fontsize=8)
        handles, labels = axes[0].get_legend_handles_labels()
        fig.legend(handles, labels, loc='upper center', ncol=len(labels), frameon=False, fontsize=7.5)
        fig.tight_layout(rect=(0, 0, 1, 0.9))
        for suffix in ('png', 'pdf'):
            fig.savefig(ROOT / f'figures/{name}.{suffix}', dpi=200)
        plt.close(fig)


def plot_width(rows, plt):
    """Height / lower bound against the largest width, n = 1000 uniform data."""
    data = summary(rows, 'wmax')
    fig, ax = plt.subplots(figsize=(4.6, 2.8))
    for algo in ('ffdh', 'nfdh', 'bfdh', 'sleator'):
        d = [data[('uniform', algo, w)] for w in WIDTHS]
        ys = [v['ratio'] for v in d]
        err = [[y - v['rmin'] for y, v in zip(ys, d)], [v['rmax'] - y for y, v in zip(ys, d)]]
        color, marker = STYLE[algo]
        ax.errorbar(WIDTHS, ys, yerr=err, color=color, marker=marker, markersize=3.5, linewidth=1.4,
                    capsize=1.5, label=LABELS[algo])
    ax.set_xlabel('largest texture width wmax (strip width W = 1000)', fontsize=8)
    ax.set_ylabel('height / lower bound', fontsize=8)
    setup_axis(ax, 'n = 1000, uniform heights')
    ax.legend(frameon=False, fontsize=7.5, loc='upper left')
    fig.tight_layout()
    for suffix in ('png', 'pdf'):
        fig.savefig(ROOT / f'figures/width.{suffix}', dpi=200)
    plt.close(fig)


def plot_example(plt):
    """Draw the packings of one small uniform input by NFDH, FFDH and Sleator."""
    from matplotlib.patches import Rectangle
    path = instance(40, 'uniform', 5, 400, 300)
    lines = path.read_text().split('\n')
    W = int(lines[0].split()[1])
    rects = [tuple(map(int, line.split())) for line in lines[1:] if line]
    fig, axes = plt.subplots(1, 3, figsize=(5.6, 4.0), sharey=True)
    for ax, algo in zip(axes, ('nfdh', 'ffdh', 'sleator')):
        out = subprocess.run([str(ROOT / 'build/pack'), algo], stdin=path.open(), capture_output=True,
                             text=True, check=True).stdout.split('\n')
        height, lb = map(int, out[0].split())
        for (w, h), line in zip(rects, out[1:]):
            x, y = map(int, line.split())
            ax.add_patch(Rectangle((x, y), w, h, facecolor=STYLE[algo][0], alpha=0.55,
                                   edgecolor='#fcfcfb', linewidth=0.8))
        ax.axhline(height, color='#52514e', linewidth=0.8, linestyle='--')
        ax.set_xlim(0, W)
        ax.set_ylim(0, 1.08 * max(height for height in [height] + [0]) + 1)
        ax.set_aspect('equal')
        setup_axis(ax, f'{LABELS[algo]}: height {height}')
        ax.grid(False)
        ax.set_xlabel('x', fontsize=8)
    axes[0].set_ylabel('y', fontsize=8)
    top = max(a.get_ylim()[1] for a in axes)
    for a in axes:
        a.set_ylim(0, top)
    fig.tight_layout()
    for suffix in ('png', 'pdf'):
        fig.savefig(ROOT / f'figures/example.{suffix}', dpi=200)
    plt.close(fig)
    print(f'example: n = 40, W = {W}, lower bound {lb}')


def tables(size_rows, width_rows):
    """Print the numbers used in the report tables."""
    data = summary(size_rows, 'n')
    print('\ntime (ms), median over 3 seeds')
    for kind in TYPES:
        for algo in ALGORITHMS:
            cells = [f"{data[(kind, algo, n)]['time']:.3g}" if (kind, algo, n) in data else '-' for n in SIZES]
            print(f'| {kind} | {algo} | ' + ' | '.join(cells) + ' |')
    print('\nheight / lower bound, mean over 3 seeds')
    for kind in TYPES:
        for algo in ('ffdh', 'nfdh', 'bfdh', 'sleator'):
            cells = [f"{data[(kind, algo, n)]['ratio']:.3f}" for n in SIZES]
            print(f'| {kind} | {algo} | ' + ' | '.join(cells) + ' |')
    data = summary(width_rows, 'wmax')
    print('\nwidth experiment: height / lower bound, mean (min-max) over 10 seeds')
    for algo in ('ffdh', 'nfdh', 'bfdh', 'sleator'):
        cells = [f"{data[('uniform', algo, w)]['ratio']:.3f} ({data[('uniform', algo, w)]['rmin']:.3f}-"
                 f"{data[('uniform', algo, w)]['rmax']:.3f})" for w in WIDTHS]
        print(f'| {algo} | ' + ' | '.join(cells) + ' |')


def main():
    """Run the two experiments (or load them), then draw the figures and print the tables."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--plot-only', action='store_true', help='redraw from data/*.json')
    args = parser.parse_args()
    if args.plot_only:
        size_rows = json.loads((DATA / 'size.json').read_text())
        width_rows = json.loads((DATA / 'width.json').read_text())
    else:
        size_rows, width_rows = run_size(), run_width()
        DATA.mkdir(exist_ok=True)
        (DATA / 'size.json').write_text(json.dumps(size_rows, indent=1) + '\n')
        (DATA / 'width.json').write_text(json.dumps(width_rows, indent=1) + '\n')
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    (ROOT / 'figures').mkdir(exist_ok=True)
    plot_size(size_rows, plt)
    plot_width(width_rows, plt)
    plot_example(plt)
    tables(size_rows, width_rows)


if __name__ == '__main__':
    main()
