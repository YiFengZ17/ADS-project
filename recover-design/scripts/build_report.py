"""Compile current Markdown into a local PDF; never modify sources/submission PDFs."""
import argparse
import hashlib
import re
import subprocess
from pathlib import Path
from markdown_it import MarkdownIt

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'report/report.md'
OUT = ROOT / 'build/md-report'

def escape(text):
    mapping = {'\\': r'\textbackslash{}', '&': r'\&', '%': r'\%', '$': r'\$',
               '#': r'\#', '_': r'\_', '{': r'\{', '}': r'\}', '~': r'\textasciitilde{}', '^': r'\textasciicircum{}'}
    return ''.join(mapping.get(c, c) for c in text)

def text_latex(text):
    # Explicit $...$ is preserved. Common complexity notation is typeset as math.
    pieces = re.split(r'(\$[^$\n]+\$|(?:O|Θ)\([^()]+\))', text)
    result = []
    for value in pieces:
        if value.startswith('$') and value.endswith('$'):
            result.append(value)
        elif re.fullmatch(r'(?:O|Θ)\([^()]+\)', value):
            result.append('$' + value.replace('Θ', r'\Theta').replace('log ', r'\log ') + '$')
        else:
            result.append(escape(value))
    return ''.join(result)

def inline(tokens):
    result = []
    for t in tokens or []:
        if t.type == 'text': result.append(text_latex(t.content))
        elif t.type in ('softbreak', 'hardbreak'): result.append(r'\par ')
        elif t.type == 'code_inline': result.append(r'\texttt{' + escape(t.content) + '}')
        elif t.type == 'em_open': result.append(r'\textit{')
        elif t.type == 'strong_open': result.append(r'\textbf{')
        elif t.type in ('em_close', 'strong_close', 'link_close'): result.append('}')
        elif t.type == 'link_open':
            url = t.attrGet('href')
            result.append(r'\href{\detokenize{' + url + '}}{')
        elif t.type == 'image':
            image = (SOURCE.parent / t.attrGet('src')).resolve()
            if not image.is_relative_to(ROOT) or not image.is_file():
                raise ValueError(f'Missing or external image: {image}')
            result.append('\n' + r'\begin{figure}[htbp]\centering' + '\n' +
                          r'\includegraphics[width=\linewidth]{\detokenize{' + image.as_posix() + '}}\n' +
                          r'\caption{' + escape(t.content) + '}\n' + r'\end{figure}' + '\n')
        else: raise ValueError(f'Unsupported inline Markdown: {t.type}')
    return ''.join(result)

def convert(markdown):
    # Protect math backslashes from Markdown's punctuation escaping.
    markdown = re.sub(r'\$[^$\n]+\$', lambda m: m.group().replace('\\', '\\\\'), markdown)
    tokens = MarkdownIt('commonmark', {'html': False}).enable('table').parse(markdown)
    body, rows, cells = [], [], []
    table = False
    before_body = True
    first_title = True
    cover_metadata = False
    for i, t in enumerate(tokens):
        typ = t.type
        if typ == 'heading_open':
            title = tokens[i+1].content
            if first_title:
                body.append(r'\begin{center}{\titlefont\bfseries\fontsize{20}{25}\selectfont ')
                first_title = False
            elif before_body and title == 'Abstract':
                body.append(r'\begin{center}\textbf{')
            else:
                if before_body:
                    body.append(r'\clearpage\begingroup\setlength{\parskip}{0pt}\tableofcontents\endgroup\clearpage')
                    before_body = False
                level = int(t.tag[1:])
                command = {1: 'section', 2: 'subsection', 3: 'subsubsection'}.get(level, 'paragraph')
                # Keep a heading with its first lines; tables reserve their own space.
                reserve = {'Correctness Tests': 23, 'Timing Results': 23, 'Limitations and Improvements': 12}.get(title, 5)
                body.append(r'\Needspace{' + str(reserve) + r'\baselineskip}' + chr(92) + command + '{')
        elif typ == 'heading_close':
            # Cover title and abstract are centered and unnumbered.
            if before_body: body.append(r'}\end{center}')
            else: body.append('}')
        elif typ == 'inline':
            value = inline(t.children)
            if table: cells.append(value)
            else: body.append(value)
        elif typ == 'paragraph_open':
            content = tokens[i+1].content if i+1 < len(tokens) else ''
            cover_metadata = before_body and content.startswith(('Author:', 'Group number:', 'Date:'))
            if cover_metadata: body.append(r'\begin{center}{\titlefont ')

        elif typ == 'paragraph_close':
            if not table:
                if cover_metadata: body.append(r'}\end{center}')
                cover_metadata = False
                body.append('\n\n')
        elif typ == 'bullet_list_open': body.append(r'\begin{itemize}')
        elif typ == 'bullet_list_close': body.append(r'\end{itemize}')
        elif typ == 'ordered_list_open':
            body.append(r'\begin{enumerate}')
            start = t.attrGet('start')
            if start: body.append(r'\setcounter{enumi}{' + str(int(start)-1) + '}')
        elif typ == 'ordered_list_close': body.append(r'\end{enumerate}')
        elif typ == 'list_item_open': body.append(r'\item ')
        elif typ == 'list_item_close': body.append('\n')
        elif typ == 'table_open': table, rows = True, []
        elif typ == 'tr_open': cells = []
        elif typ == 'tr_close': rows.append(cells)
        elif typ == 'table_close':
            columns = len(rows[0])
            widths = [1/columns]*columns
            if columns == 2: widths = [.25, .75]
            if columns == 3: widths = [.32, .19, .49]
            spec = ''.join(r'>{\raggedright\arraybackslash}p{' + str(round(.87*w, 4)) + r'\linewidth}' for w in widths)
            body.append(r'\Needspace{' + str(min(32, 1.5*len(rows)+2)) + r'\baselineskip}')
            body.append(r'\begin{longtable}{' + spec + '}' + '\n' + r'\toprule')
            for n, row in enumerate(rows):
                body.append(' & '.join(row) + r' \\')
                if n == 0: body.append(r'\midrule\endhead')
            body.append(r'\bottomrule\end{longtable}')
            table = False
        elif typ in ('thead_open','thead_close','tbody_open','tbody_close','th_open','th_close','td_open','td_close'): pass
        elif typ in ('fence', 'code_block'):
            if r'\end{lstlisting}' in t.content: raise ValueError('Invalid listing terminator')
            body.append(r'\Needspace{' + str(min(28, len(t.content.splitlines()) + 2)) + r'\baselineskip}')
            body.append(r'\begin{lstlisting}[basicstyle=\ttfamily\small,breaklines=true]' + '\n' + t.content + r'\end{lstlisting}')
        elif typ == 'hr': body.append(r'\par\noindent\rule{\linewidth}{0.4pt}')
        elif typ == 'blockquote_open': body.append(r'\begin{quote}')
        elif typ == 'blockquote_close': body.append(r'\end{quote}')
        else: raise ValueError(f'Unsupported Markdown: {typ}')
    return '\n'.join(body)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--compile', action='store_true')
    parser.add_argument('--anonymous', action='store_true')
    args = parser.parse_args()
    original = SOURCE.read_bytes()
    OUT.mkdir(parents=True, exist_ok=True)
    tex = OUT / ('report-anonymous.tex' if args.anonymous else 'report-from-md.tex')
    markdown = original.decode()
    if args.anonymous:
        markdown = markdown.replace('YiFeng Zheng', 'Author').replace('YuJie Sun', 'Reviewer')
        markdown = markdown.replace('Group number: 17', 'Group number: withheld')
    style = (ROOT / 'scripts/report-style.tex').read_text()
    tex.write_text(style + '\n' + r'\begin{document}' + '\n' + convert(markdown) + '\n' + r'\end{document}' + '\n')
    if args.compile:
        with (OUT / ('compile-anonymous.log' if args.anonymous else 'compile.log')).open('w') as log:
            for _ in range(3):
                subprocess.run(['xelatex', '-halt-on-error', '-interaction=nonstopmode', '-no-shell-escape',
                                f'-output-directory={OUT}', str(tex)], cwd=ROOT, stdout=log, stderr=subprocess.STDOUT, check=True)
        assert SOURCE.read_bytes() == original, 'Markdown changed during build'
        import shutil
        destination = ROOT / 'build' / ('recover-design-anonymous.pdf' if args.anonymous else 'recover-design.pdf')
        shutil.copy2(tex.with_suffix('.pdf'), destination)
        print(f'Built {destination}; Markdown SHA256={hashlib.sha256(original).hexdigest()}')
        print('Review PDF before explicitly updating TA; prepare and inspect an anonymous version separately.')
    else: print(tex)

if __name__ == '__main__': main()
