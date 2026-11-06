"""Print derived numbers for the report: time per unit of work and growth between sizes."""
import json
import math
from pathlib import Path
import sys

from benchmark import DATA, EXPERIMENTS, summary


def main():
    names = sys.argv[1:] or list(EXPERIMENTS)
    for name in names:
        path = DATA / f'{name}.json'
        if not path.exists():
            continue
        exp = EXPERIMENTS[name]
        data = summary(json.loads(path.read_text()), exp)
        print(f'\n== {name} (vary {exp["vary"]}) ==')
        for kind in exp['types']:
            for algorithm in exp['algorithms']:
                points = [(v, data[(kind, algorithm, v)]) for v in exp['values'] if (kind, algorithm, v) in data]
                if not points:
                    continue
                parts = []
                for i, (v, d) in enumerate(points):
                    ns = d['median'] * 1e6 / d['work'] if d['work'] else float('nan')
                    text = f"{v}: {d['median']:.3g} ms, work {d['work']:.3g}, {ns:.3g} ns/unit, c {d['c']:.3g}"
                    if i:
                        v0, d0 = points[i - 1]
                        slope = math.log(d['median'] / d0['median']) / math.log(v / v0)
                        text += f', slope {slope:.2f}'
                    parts.append(text)
                print(f'{kind:13s} {algorithm:13s}\n    ' + '\n    '.join(parts))


if __name__ == '__main__':
    main()
