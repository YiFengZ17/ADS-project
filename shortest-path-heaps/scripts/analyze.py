"""Summarize local per-query CSVs and draw the reproducible report figure."""
import csv
import math
import statistics
from pathlib import Path
import matplotlib.pyplot as plt
ROOT = Path(__file__).resolve().parents[1]
rows=[]
distribution=[]
# Interpolate between adjacent sorted observations, including both endpoints.
def percentile(values, percentage):
    ordered=sorted(values)
    position=(len(ordered)-1)*percentage/100
    low=math.floor(position)
    high=math.ceil(position)
    return ordered[low]+(ordered[high]-ordered[low])*(position-low)

for name in ('NY','BAY','COL'):
    path=ROOT/'data'/f'results-{name}.csv'
    with path.open() as f: data=list(csv.DictReader(f))
    assert len(data)==2000
    by={m:[r for r in data if r['method']==m] for m in ('binary','fibonacci')}
    assert all(len(v)==1000 for v in by.values())
    assert all(a['distance']==b['distance'] and a['source']==b['source'] and a['target']==b['target'] for a,b in zip(by['binary'],by['fibonacci']))
    for method, group in by.items():
        times=[float(r['seconds'])*1000 for r in group]
        rows.append(dict(dataset=name,method=method,nodes=int(group[0]['nodes']),arcs=int(group[0]['arcs']),
                         mean_ms=statistics.mean(times),median_ms=statistics.median(times),
                         total_s=sum(times)/1000,unreachable=sum(r['distance']=='INF' for r in group)))
        distribution.append(dict(dataset=name,method=method,p25_ms=percentile(times,25),
                                 p95_ms=percentile(times,95)))
with (ROOT/'data'/'summary.csv').open('w',newline='') as f:
    w=csv.DictWriter(f,fieldnames=rows[0].keys()); w.writeheader(); w.writerows(rows)
with (ROOT/'data'/'distribution.csv').open('w',newline='') as f:
    w=csv.DictWriter(f,fieldnames=distribution[0].keys()); w.writeheader(); w.writerows(distribution)
print('dataset method nodes arcs mean_ms median_ms total_s unreachable')
for r in rows: print(*(f'{v:.3f}' if isinstance(v,float) else v for v in r.values()))
fig,ax=plt.subplots(figsize=(6.5,3.7))
for method,label,color in [('binary','Binary heap','#1f77b4'),('fibonacci','Fibonacci heap','#d95f02')]:
    x=[r['nodes'] for r in rows if r['method']==method]
    y=[r['mean_ms'] for r in rows if r['method']==method]
    ax.plot(x,y,'o-',label=label,color=color,linewidth=2)
for r in rows:
    if r['method']=='binary': ax.annotate(r['dataset'],(r['nodes'],r['mean_ms']),xytext=(3,5),textcoords='offset points',fontsize=9)
ax.set(xlabel='Vertices in road network',ylabel='Mean time per query (ms)')
ax.grid(alpha=.25);ax.legend();fig.tight_layout()
fig.savefig(ROOT/'figures'/'runtime-vs-size.pdf')
fig.savefig(ROOT/'figures'/'runtime-vs-size.png',dpi=180)
