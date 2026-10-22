"""Synchronize source and README into clean submission snapshots; preserve PDFs."""
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]


def main():
    """Reject unexpected entries, then copy only the solver and submission README."""
    packages = [('TA', 'yds_2026G17_P3.pdf'), ('Peer Review', 'recover-design.pdf')]
    for name, pdf_name in packages:
        target = ROOT / name
        extras = {p.name for p in target.iterdir()} - {'code', 'README.md', pdf_name}
        code_extras = {p.name for p in (target / 'code').iterdir()} - {'main.cpp'}
        if extras or code_extras:
            raise RuntimeError(f'Unexpected submission entries in {name}: {sorted(extras | code_extras)}')
    for name, _ in packages:
        target = ROOT / name
        shutil.copy2(ROOT / 'code/main.cpp', target / 'code/main.cpp')
        shutil.copy2(ROOT / 'scripts/submission-readme.md', target / 'README.md')
    print('Updated clean TA and Peer Review source/README snapshots; PDFs preserved.')


if __name__ == '__main__':
    main()
