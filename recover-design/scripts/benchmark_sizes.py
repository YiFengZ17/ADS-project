"""Time the solver on generator inputs of growing size and draw the report figure."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess
import time

from check import validate

ROOT = Path(__file__).resolve().parents[1]
SIZES = (10, 20, 30, 40, 50)
SEEDS = (1, 2, 3, 4, 5)
# (label, generator type, connector density)
KINDS = (('planted, density 1.0', 'planted', '1.0'),
         ('planted, density 0.5', 'planted', '0.5'),
         ('broken, density 1.0', 'broken', '1.0'),
         ('random, density 0.6', 'random', '0.6'))


def run_once(binary, text):
    """Run the solver once and return its output and wall time in milliseconds."""
    start = time.perf_counter()
    result = subprocess.run([str(binary)], input=text, text=True, capture_output=True, timeout=60)
    elapsed = (time.perf_counter() - start) * 1000
    assert result.returncode == 0 and not result.stderr, result.stderr
    return result.stdout, elapsed


def main():
    """Generate each input with code/gen.cpp, check the answer, then time 1 warmup + 10 runs."""
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/recover-design')
    args = parser.parse_args()
    gen = ROOT / 'build/gen'
    subprocess.run(['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', '-Wpedantic',
                    str(ROOT / 'code/gen.cpp'), '-o', str(gen)], check=True)
    results = []
    for label, kind, density in KINDS:
        for n in SIZES:
            medians, solved, per_input = [], 0, []
            for seed in SEEDS:
                text = subprocess.run([str(gen), str(n), str(n), density, kind, str(seed)],
                                      capture_output=True, text=True, check=True).stdout
                values = list(map(int, text.split()))
                grid = [values[2 + r * n:2 + (r + 1) * n] for r in range(n)]
                output, _ = run_once(args.binary, text)  # warmup, not measured
                has_answer = output != 'No Solution\n'
                if has_answer:
                    validate(grid, output, True)
                    solved += 1
                elif kind == 'planted':
                    raise AssertionError(f'No Solution for planted input {n} {density} {seed}')
                times = [run_once(args.binary, text)[1] for _ in range(10)]
                medians.append(statistics.median(times))
                per_input.append({'seed': seed, 'solved': has_answer, 'median_ms': medians[-1]})
            row = {'kind': label, 'n': n, 'solved': solved, 'inputs': len(SEEDS),
                   'median_ms': statistics.median(medians), 'min_ms': min(medians), 'max_ms': max(medians), 'per_input': per_input}
            results.append(row)
            print(f"{label:22s} {n}x{n}: median {row['median_ms']:.2f} ms, "
                  f"range {row['min_ms']:.2f}-{row['max_ms']:.2f} ms, solved {solved}/{len(SEEDS)}", flush=True)
    dest = ROOT / 'data/size-timings.json'
    dest.parent.mkdir(exist_ok=True)
    dest.write_text(json.dumps(results, indent=2) + '\n')
    plot(results)


def plot(results):
    """Draw median time against grid size, one line per input kind."""
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    colors = ['#2a78d6', '#eb6834', '#1baf7a', '#eda100']
    markers = ['o', 's', '^', 'D']
    fig, ax = plt.subplots(figsize=(6.4, 3.8))
    for (label, _, _), color, marker in zip(KINDS, colors, markers):
        rows = [r for r in results if r['kind'] == label]
        x = [r['n'] for r in rows]
        y = [r['median_ms'] for r in rows]
        err = [[r['median_ms'] - r['min_ms'] for r in rows], [r['max_ms'] - r['median_ms'] for r in rows]]
        ax.errorbar(x, y, yerr=err, color=color, marker=marker, markersize=6, linewidth=2,
                    capsize=3, label=label)
    ax.set_yscale('log')
    ax.set_xlabel('Grid size n (input is n by n)')
    ax.set_ylabel('Median time per run (ms, log scale)')
    ax.set_xticks(SIZES)
    ax.grid(True, which='major', color='#e4e4e0', linewidth=0.8)
    for side in ('top', 'right'):
        ax.spines[side].set_visible(False)
    ax.legend(frameon=False, fontsize=9)
    fig.tight_layout()
    for suffix in ('png', 'pdf'):
        fig.savefig(ROOT / f'figures/runtime-vs-size.{suffix}', dpi=200)


if __name__ == '__main__':
    main()
