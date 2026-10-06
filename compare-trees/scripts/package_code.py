"""Synchronize verified development sources into both submission snapshots."""
from pathlib import Path
import shutil
root = Path(__file__).resolve().parents[1]
for target in ('TA', 'Peer Review'):
    destination = root / target / 'code'
    destination.mkdir(parents=True, exist_ok=True)
    for source in (root / 'code').glob('*.c'):
        shutil.copy2(source, destination / source.name)
print('Code synchronized. Review TA identity and Peer Review anonymity before updating PDFs.')
