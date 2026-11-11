"""Synchronize source and README into clean submission snapshots; preserve PDFs."""
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ('main.cpp', 'gen.cpp')  # solver (with its timing mode) and test data generator


def main():
    """Reject unexpected entries, then copy only the source files and the submission README."""
    packages = [('TA', 'yds_2026G17_P5.pdf'), ('Peer Review', 'shopping-coupons.pdf')]
    for name, pdf_name in packages:
        target = ROOT / name
        (target / 'code').mkdir(parents=True, exist_ok=True)
        extras = {p.name for p in target.iterdir()} - {'code', 'README.md', pdf_name}
        code_extras = {p.name for p in (target / 'code').iterdir()} - set(SOURCES)
        if extras or code_extras:
            raise RuntimeError(f'Unexpected submission entries in {name}: {sorted(extras | code_extras)}')
    for name, _ in packages:
        target = ROOT / name
        for source in SOURCES:
            shutil.copy2(ROOT / 'code' / source, target / 'code' / source)
        shutil.copy2(ROOT / 'scripts/submission-readme.md', target / 'README.md')
    print('Updated clean TA and Peer Review source/README snapshots; PDFs preserved.')


if __name__ == '__main__':
    main()
