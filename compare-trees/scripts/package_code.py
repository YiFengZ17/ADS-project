"""Synchronize verified development sources into both submission snapshots."""
from pathlib import Path
import shutil
root = Path(__file__).resolve().parents[1]
for target, pdf_name in [('TA', 'yds_2026G17_P1.pdf'), ('Peer Review', 'compare-search-trees.pdf')]:
    package = root / target
    extras = {p.name for p in package.iterdir()} - {'code', 'README.md', pdf_name}
    code_extras = {p.name for p in (package / 'code').iterdir()} - {p.name for p in (root / 'code').glob('*.c')}
    if extras or code_extras:
        raise RuntimeError(f'Unexpected submission entries in {target}: {sorted(extras | code_extras)}')
for target in ('TA', 'Peer Review'):
    destination = root / target / 'code'
    destination.mkdir(parents=True, exist_ok=True)
    for source in (root / 'code').glob('*.c'):
        shutil.copy2(source, destination / source.name)
print('Code synchronized. Review TA identity and Peer Review anonymity before updating PDFs.')
