"""Draw report figures from checked ten-run data without changing report text."""
import csv
import math
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from analyze import ROOT, SCENARIOS, SIZES, TREES, PHASES, validate, summarize

COLORS = {'BST': '#666666', 'AVL': '#2374ab', 'Splay': '#b27400',
          'RedBlack': '#c65378', 'BPlus32': '#548235'}
MARKERS = {'BST': 'o', 'AVL': 's', 'Splay': '^', 'RedBlack': 'D', 'BPlus32': 'v'}
TITLES = {'increasing_same': 'Increasing insertion / increasing deletion',
          'increasing_reverse': 'Increasing insertion / reverse deletion',
          'random_random': 'Random insertion / random deletion'}


def main():
    """Use common phase scales across scenarios and show the full measured range."""
    with (ROOT / 'data/results/raw.csv').open(newline='') as f:
        rows = list(summarize(validate(csv.DictReader(f))))
    data = {tuple(row[:4]): [v / 1000 for v in row[4:]] for row in rows}
    limits = {}
    for phase in PHASES:
        values = [v for key, stats in data.items() if key[3] == phase for v in stats[1:]]
        limits[phase] = (10 ** math.floor(math.log10(min(values))),
                         10 ** math.ceil(math.log10(max(values))))
    plt.rcParams.update({'font.family': 'DejaVu Sans', 'font.size': 10,
                         'axes.spines.top': False, 'axes.spines.right': False,
                         'axes.linewidth': .7, 'pdf.fonttype': 42})
    output = ROOT / 'figures'
    output.mkdir(exist_ok=True)
    for scenario in SCENARIOS:
        fig, axes = plt.subplots(1, 3, figsize=(9, 4.3))
        for ax, phase in zip(axes, PHASES):
            for tree in TREES:
                stats = [data[(scenario, n, tree, phase)] for n in SIZES]
                med = [s[0] for s in stats]
                errors = [[s[0] - s[1] for s in stats], [s[2] - s[0] for s in stats]]
                ax.errorbar(SIZES, med, yerr=errors, label=tree, color=COLORS[tree],
                            marker=MARKERS[tree], markersize=3.5, linewidth=1,
                            elinewidth=.7, capsize=2)
            ax.set(xscale='log', yscale='log', ylim=limits[phase],
                   title=phase.capitalize(), xlabel='N (keys)', ylabel='Time (s)')
            ax.grid(True, which='major', color='#dddddd', linewidth=.5)
            ax.tick_params(which='both', labelsize=9)
        handles, labels = axes[0].get_legend_handles_labels()
        fig.suptitle(TITLES[scenario], y=.98, fontsize=11)
        fig.legend(handles, labels, loc='upper center', bbox_to_anchor=(.5, .91),
                   ncol=5, frameon=False)
        fig.subplots_adjust(left=.08, right=.985, bottom=.18, top=.70, wspace=.39)
        for suffix in ('png', 'pdf'):
            fig.savefig(output / f'{scenario}.{suffix}', dpi=220)
        plt.close(fig)
    print('Updated three PNG/PDF figures: ten-run medians, min-max error bars.')


if __name__ == '__main__':
    main()
