"""Sync verified development sources and PDFs into both submission folders."""
from pathlib import Path
import shutil
import subprocess

ROOT = Path(__file__).resolve().parents[1]
COMMON = [Path('Makefile'), Path('code/main.cpp'), Path('scripts/analyze.py'),
          Path('scripts/test_parser.py')]
PACKAGES = [
    ('TA', ROOT/'build/shortest-path-heaps.pdf', 'yds_2026G17_P2.pdf'),
    ('Peer Review', ROOT/'build/shortest-path-heaps-anonymous.pdf', 'shortest-path-heaps.pdf'),
]


def main():
    source = (ROOT/'report/report.md').read_bytes()
    if not source:
        raise RuntimeError('empty report source')
    for folder, pdf, pdf_name in PACKAGES:
        if not pdf.is_file():
            raise RuntimeError(f'missing checked PDF: {pdf}')
        destination = ROOT/folder
        for relative in COMMON:
            target = destination/relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(ROOT/relative, target)
        shutil.copy2(ROOT/'scripts/submission-readme.md', destination/'README.md')
        shutil.copy2(pdf, destination/pdf_name)
    anonymous = subprocess.check_output(['pdftotext', str(PACKAGES[1][1]), '-']).decode()
    if any(value in anonymous for value in ('YiFeng Zheng', 'YuJie Sun', 'Group 17',
                                             'Division of Work', 'Declaration', 'Signatures')):
        raise RuntimeError('identifying content in anonymous PDF')
    for relative in COMMON:
        for folder, _, _ in PACKAGES:
            if (ROOT/relative).read_bytes() != (ROOT/folder/relative).read_bytes():
                raise RuntimeError(f'package source differs: {folder}/{relative}')
    print('PASS: named and anonymous packages synchronized')


if __name__ == '__main__':
    main()
