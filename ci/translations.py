#!/usr/bin/env python3
"""list tele's translatable strings and what each translation lacks.

run against a tdesktop tree with the patches applied as commits (`tele.py checkout` or ci's `apply`):

    python ci/translations.py --tdesktop ../tdesktop
    python ci/translations.py --tdesktop ../tdesktop --todo out/   # out/<lang>.missing.json for translators

strings are keyed by their english text, as `Tele::Translate` looks them up. tele's own files are scanned
whole, upstream files only in the lines the patches add. anything that looks like ui text counts, so a few
keys never reach the screen; a missing translation just shows english."""

import argparse
import json
import pathlib
import re
import subprocess

HERE = pathlib.Path(__file__).resolve().parent
TRANSLATIONS = 'Telegram/Resources/tele/translations'
SOURCES = 'Telegram/SourceFiles'
LITERAL = re.compile(r'u"((?:[^"\\]|\\.)*)"((?:\s*"(?:[^"\\]|\\.)*")*)_q')
PIECE = re.compile(r'"((?:[^"\\]|\\.)*)"')
OPTION_TEXT = re.compile(r'\.(?:name|description)\s*=\s*((?:"(?:[^"\\]|\\.)*"\s*)+)')
PLURAL = re.compile(r'Plural\(\s*' + LITERAL.pattern + r'\s*,\s*' + LITERAL.pattern)
PAIR = re.compile(r'[(,]\s*' + LITERAL.pattern + r'\s*,\s*' + LITERAL.pattern)
SKIP_LINE = re.compile(r'\b(?:LOG|DEBUG_LOG|TELE_LOG)\s*\(\(|\bqWarning\b|\bqDebug\b')
IDENTIFIER = re.compile(r'^[a-z0-9_.:/#?=&%{}\[\]$@*+\\-]+$')


def git(repo, *args):
    return subprocess.run(['git', '-C', str(repo), *args], check=True, capture_output=True,
                          text=True, encoding='utf-8', errors='replace').stdout


def unescape(text):
    def replace(match):
        code = match.group(1)
        if code.startswith('u'):
            return chr(int(code[1:], 16))
        return {'n': '\n', 't': '\t', '"': '"', '\\': '\\', "'": "'"}.get(code, match.group(0))
    return re.sub(r'\\(u[0-9a-fA-F]{4}|.)', replace, text)


def joined(first, rest):
    return unescape(first + ''.join(PIECE.findall(rest)))


def looks_like_ui(text):
    if not re.search(r'[A-Za-z]', text) or '://' in text or text.startswith(('tele-', 'tg:')):
        return False
    if IDENTIFIER.match(text) or re.match(r'^[A-Z0-9_]+$', text):
        return False
    # markup and the scripting api's own messages (they are part of its english api)
    if text.lstrip().startswith(('@', '.', '<', '#', 'tg.', 'fetch:')) or ('{' in text and '}' in text and ';' in text):
        return False
    return ' ' in text or text[0].isupper() or re.search(r'%\d', text) is not None


def plural_pair(one, other):
    placeholders = re.compile(r'%\d')
    if one == other or sorted(placeholders.findall(one)) != sorted(placeholders.findall(other)):
        return False
    if not placeholders.search(one):
        return False
    a, b = one.split(' '), other.split(' ')
    if len(a) != len(b):
        return False
    changed = [(x, y) for x, y in zip(a, b) if x != y]
    return bool(changed) and all(
        y.lower().startswith(x.lower().rstrip('.,?!:')[:3]) for x, y in changed)


def scan(text):
    lines = [line for line in text.splitlines() if not SKIP_LINE.search(line)]
    body = '\n'.join(lines)
    plurals = {}
    for match in PLURAL.finditer(body):
        plurals[joined(match.group(3), match.group(4))] = joined(match.group(1), match.group(2))
    # helpers that take the two forms and call Plural themselves: "%1 gift pinned", "%1 gifts pinned"
    for match in PAIR.finditer(body):
        one, other = joined(match.group(1), match.group(2)), joined(match.group(3), match.group(4))
        if plural_pair(one, other):
            plurals.setdefault(other, one)
    singulars = set(plurals.values())
    strings = set()
    for match in LITERAL.finditer(body):
        value = joined(match.group(1), match.group(2))
        if looks_like_ui(value) and value not in plurals and value not in singulars:
            strings.add(value)
    for match in OPTION_TEXT.finditer(body):
        value = unescape(''.join(PIECE.findall(match.group(1))))
        if value:
            strings.add(value)
    return strings, plurals


def collect(tdesktop, upstream):
    # {english: file it was first found in}, {english plural: {'one': singular, 'file': ...}}
    names = git(tdesktop, 'diff', '--name-only', upstream, '--', SOURCES).split()
    names += git(tdesktop, 'ls-files', '--others', '--exclude-standard', '--', SOURCES).split()
    strings, plurals = {}, {}
    for name in sorted(names):
        if not name.endswith(('.cpp', '.h', '.mm')):
            continue
        path = tdesktop / name
        if not path.exists():
            continue
        new = subprocess.run(['git', '-C', str(tdesktop), 'cat-file', '-e', f'{upstream}:{name}'],
                             capture_output=True).returncode != 0
        if new:
            text = path.read_text(encoding='utf-8', errors='replace')
        else:
            diff = git(tdesktop, 'diff', '-U0', upstream, '--', name)
            text = '\n'.join(line[1:] for line in diff.splitlines()
                             if line.startswith('+') and not line.startswith('+++'))
        found, plural = scan(text)
        short = name[len(SOURCES) + 1:]
        for value in found:
            strings.setdefault(value, short)
        for other, one in plural.items():
            plurals.setdefault(other, {'one': one, 'file': short})
    for other, plural in plurals.items():
        strings.pop(other, None)
        strings.pop(plural['one'], None)
    return strings, plurals


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--tdesktop', required=True, type=pathlib.Path)
    parser.add_argument('--upstream', default='', help='upstream tag, default: the UPSTREAM file')
    parser.add_argument('--todo', type=pathlib.Path, help='write <lang>.missing.json here')
    parser.add_argument('--list', type=pathlib.Path, help='write every translatable string here as json')
    args = parser.parse_args()
    upstream = args.upstream or (HERE.parent / 'UPSTREAM').read_text().strip()
    strings, plurals = collect(args.tdesktop, upstream)
    print(f'{len(strings)} strings and {len(plurals)} plurals to translate')
    if args.list:
        args.list.write_text(json.dumps({'strings': dict(sorted(strings.items())),
                                         'plurals': dict(sorted(plurals.items()))},
                                        ensure_ascii=False, indent=1), encoding='utf-8')
    for table in sorted((args.tdesktop / TRANSLATIONS).glob('*.json')):
        data = json.loads(table.read_text(encoding='utf-8'))
        entries = data.get('strings', {})
        missing = {s: f for s, f in sorted(strings.items()) if not isinstance(entries.get(s), str)}
        missing_plurals = {p: v for p, v in sorted(plurals.items()) if not isinstance(entries.get(p), dict)}
        stale = sorted(k for k in entries if k not in strings and k not in plurals)
        print(f'{table.stem}: {len(entries)} entries, {len(missing)} strings and '
              f'{len(missing_plurals)} plurals missing, {len(stale)} stale')
        if args.todo:
            args.todo.mkdir(parents=True, exist_ok=True)
            (args.todo / f'{table.stem}.missing.json').write_text(json.dumps(
                {'strings': missing, 'plurals': missing_plurals, 'stale': stale},
                ensure_ascii=False, indent=1), encoding='utf-8')


if __name__ == '__main__':
    main()
