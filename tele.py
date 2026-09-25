#!/usr/bin/env python3
"""Keep a queue of patches on top of Telegram Desktop releases.

This repository holds:
  UPSTREAM                      the tdesktop release tag the patches target, e.g. v7.2.9
  patches/tdesktop/*.patch      patches for the tdesktop repository itself
  patches/<submodule>/*.patch   patches for a submodule, e.g. patches/Telegram/lib_ui/

Commands (the tdesktop checkout defaults to ../tdesktop next to this repository):
  checkout [TAG]  move tdesktop and all submodules to TAG (default: UPSTREAM)
                  and apply the patches as commits on a `tele` branch
  continue        resume applying after resolving a conflict with git am
  export          turn the commits on top of TAG back into patches/ and set UPSTREAM to TAG
  apply           apply the patches to a fresh clone that is already on the right tag (CI)
"""

import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

UPSTREAM_URL = 'https://github.com/telegramdesktop/tdesktop.git'
ROOT_GROUP = 'tdesktop'
BRANCH = 'tele'

HERE = Path(__file__).resolve().parent
PATCHES = HERE / 'patches'
UPSTREAM_FILE = HERE / 'UPSTREAM'

# Stable output so re-exporting unchanged commits produces byte-identical files.
FORMAT_PATCH_ARGS = ['--zero-commit', '--no-signature', '--no-numbered', '--full-index']
# --keep-cr: patch files are stored verbatim (see .gitattributes), so a CR in them is real content.
AM_ARGS = ['am', '--3way', '--keep-cr']


class Fail(Exception):
    pass


def run_git(cwd, *args):
    return subprocess.run(['git', *args], cwd=cwd, capture_output=True, text=True,
                          encoding='utf-8', errors='replace')


def git(cwd, *args):
    result = run_git(cwd, *args)
    if result.returncode != 0:
        raise Fail(f'git {" ".join(args)} failed in {cwd}:\n{(result.stderr + result.stdout).strip()}')
    return result.stdout.strip()


def write_text(path, text):
    path.write_text(text, encoding='utf-8', newline='\n')


def read_upstream():
    if not UPSTREAM_FILE.is_file():
        raise Fail(f'{UPSTREAM_FILE} is missing')
    return UPSTREAM_FILE.read_text(encoding='utf-8').strip()


def patch_groups():
    """{group: [patch files]}, group being 'tdesktop' or a submodule path."""
    groups = {}
    if PATCHES.is_dir():
        for patch in sorted(PATCHES.rglob('*.patch')):
            group = patch.parent.relative_to(PATCHES).as_posix()
            if group == '.':
                raise Fail(f'{patch.name}: put patches in patches/{ROOT_GROUP}/ or patches/<submodule>/')
            groups.setdefault(group, []).append(patch)
    return groups


def repo_path(tdesktop, group):
    return tdesktop if group == ROOT_GROUP else tdesktop / group


def is_repo(path):
    return (path / '.git').exists()


def submodule_paths(tdesktop):
    paths = []
    for line in git(tdesktop, 'submodule', 'status', '--recursive').splitlines():
        match = re.match(r'^.[0-9a-f]+ (.+?)(?: \(.*\))?$', line)
        if not match:
            raise Fail(f'unexpected `git submodule status` line: {line}')
        paths.append(match.group(1))
    return paths


def current_heads(tdesktop):
    heads = {ROOT_GROUP: git(tdesktop, 'rev-parse', 'HEAD')}
    for path in submodule_paths(tdesktop):
        if is_repo(tdesktop / path):
            heads[path] = git(tdesktop / path, 'rev-parse', 'HEAD')
    return heads


def state_file(tdesktop):
    return Path(git(tdesktop, 'rev-parse', '--absolute-git-dir')) / 'tele-state.json'


def load_state(tdesktop):
    path = state_file(tdesktop)
    return json.loads(path.read_text(encoding='utf-8')) if path.is_file() else None


def save_state(tdesktop, state):
    write_text(state_file(tdesktop), json.dumps(state, indent=2) + '\n')


def rebase_apply_dir(repo):
    path = Path(git(repo, 'rev-parse', '--git-path', 'rebase-apply'))
    return path if path.is_absolute() else repo / path


def am_in_progress(repo):
    return rebase_apply_dir(repo).is_dir()


def is_dirty(repo):
    return bool(git(repo, 'status', '--porcelain', '--untracked-files=no', '--ignore-submodules=all'))


def ignore_submodule_changes(tdesktop):
    """Keep `git status`/`git commit -a` in a parent repo from picking up moved submodule pointers."""
    for repo in [tdesktop, *(tdesktop / p for p in submodule_paths(tdesktop))]:
        if not (repo / '.gitmodules').is_file():
            continue
        names = run_git(repo, 'config', '-f', '.gitmodules', '--name-only',
                        '--get-regexp', r'^submodule\..*\.path$').stdout.split()
        for name in names:
            git(repo, 'config', name[:-len('.path')] + '.ignore', 'all')


def conflict_report(tdesktop, group, patches, am_output):
    repo = repo_path(tdesktop, group)
    prefix = '' if group == ROOT_GROUP else group + '/'
    lines = [f'Patches for {group} do not apply.']
    next_file = rebase_apply_dir(repo) / 'next'
    if next_file.is_file():
        index = int(next_file.read_text().strip())
        lines.append(f'Failed patch ({index}/{len(patches)}): patches/{group}/{patches[index - 1].name}')
    conflicted = git(repo, 'diff', '--name-only', '--diff-filter=U').splitlines()
    if conflicted:
        lines += ['Conflicted files:', *(f'  {prefix}{f}' for f in conflicted)]
    lines += ['', 'git am output:', *('  ' + line for line in am_output.strip().splitlines()), '',
              f'To resolve: cd {repo}',
              '  fix the files, `git add` them, `git am --continue` (or `git am --skip` to drop the patch),',
              f'  then run `python {Path(__file__).name} continue`.']
    return '\n'.join(lines)


def apply_pending(tdesktop, state):
    groups = patch_groups()
    while state['pending']:
        group = state['pending'][0]
        patches = groups[group]
        result = run_git(repo_path(tdesktop, group), *AM_ARGS, *map(str, patches))
        if result.returncode != 0:
            save_state(tdesktop, state)
            raise Fail(conflict_report(tdesktop, group, patches, result.stdout + result.stderr))
        print(f'applied {len(patches)} patch(es) to {group}')
        state['pending'].pop(0)
        save_state(tdesktop, state)
    state['heads'] = current_heads(tdesktop)
    save_state(tdesktop, state)


def start_applying(tdesktop, tag):
    groups = patch_groups()
    for group in groups:
        if not is_repo(repo_path(tdesktop, group)):
            raise Fail(f'patches/{group}: {group} is not a submodule of tdesktop {tag}')
    ignore_submodule_changes(tdesktop)
    state = {'tag': tag, 'bases': current_heads(tdesktop), 'pending': list(groups), 'heads': {}}
    save_state(tdesktop, state)
    apply_pending(tdesktop, state)
    print(f'tdesktop is on {tag} with {sum(map(len, groups.values()))} patch(es) applied')


def all_repos(tdesktop):
    repos = {ROOT_GROUP: tdesktop, **{p: tdesktop / p for p in submodule_paths(tdesktop)}}
    return {group: repo for group, repo in repos.items() if is_repo(repo)}


def ensure_nothing_to_lose(tdesktop, state):
    problems = []
    if state and state['pending']:
        problems.append(f'patches for {state["pending"][0]} are still being applied (use `continue`)')
    for group, repo in all_repos(tdesktop).items():
        expected = state['heads'].get(group) if state else None
        if am_in_progress(repo):
            problems.append(f'{group}: git am is in progress')
        elif is_dirty(repo):
            problems.append(f'{group}: uncommitted changes')
        elif expected and expected != git(repo, 'rev-parse', 'HEAD'):
            problems.append(f'{group}: commits that are not exported yet (use `export`)')
    if problems:
        raise Fail('refusing to move tdesktop, work would be lost:\n  ' + '\n  '.join(problems)
                   + '\npass --force to discard it')


def cmd_checkout(args):
    tdesktop = args.tdesktop
    tag = args.tag or read_upstream()
    force = ['--force'] if args.force else []
    if args.force:
        for repo in all_repos(tdesktop).values():
            if am_in_progress(repo):
                git(repo, 'am', '--quit')
    else:
        ensure_nothing_to_lose(tdesktop, load_state(tdesktop))
    print(f'fetching {tag}')
    git(tdesktop, 'fetch', '--no-tags', UPSTREAM_URL, f'+refs/tags/{tag}:refs/tags/{tag}')
    git(tdesktop, 'checkout', '-q', *force, '-B', BRANCH, f'refs/tags/{tag}')
    print('updating submodules')
    git(tdesktop, 'submodule', 'sync', '-q', '--recursive')
    git(tdesktop, 'submodule', 'update', '-q', '--init', '--recursive', *force)
    for path in submodule_paths(tdesktop):
        git(tdesktop / path, 'checkout', '-q', '-B', BRANCH)
    start_applying(tdesktop, tag)


def cmd_apply(args):
    tdesktop = args.tdesktop
    tag = run_git(tdesktop, 'describe', '--tags', '--exact-match', 'HEAD').stdout.strip()
    start_applying(tdesktop, tag or read_upstream())


def cmd_continue(args):
    tdesktop = args.tdesktop
    state = load_state(tdesktop)
    if not state or not state['pending']:
        raise Fail('nothing to continue')
    group = state['pending'][0]
    repo = repo_path(tdesktop, group)
    if am_in_progress(repo):
        raise Fail(f'git am is still in progress in {repo}: resolve, `git add`, `git am --continue` '
                   '(or `git am --skip`), then run continue again')
    if git(repo, 'rev-parse', 'HEAD') == state['bases'][group]:
        print(f'warning: no patches ended up applied to {group} (all skipped, or git am was aborted)')
    state['pending'].pop(0)
    save_state(tdesktop, state)
    apply_pending(tdesktop, state)
    print(f'all patches applied on top of {state["tag"]}')


def cmd_export(args):
    tdesktop = args.tdesktop
    state = load_state(tdesktop)
    if not state:
        raise Fail(f'{tdesktop} was not set up by `checkout`')
    if state['pending']:
        raise Fail(f'patches for {state["pending"][0]} are still being applied (use `continue`)')
    submodules = [group for group in state['bases'] if group != ROOT_GROUP]
    changed = {}
    for group, base in state['bases'].items():
        repo = repo_path(tdesktop, group)
        if not is_repo(repo):
            continue
        if am_in_progress(repo):
            raise Fail(f'{group}: git am is in progress')
        if is_dirty(repo):
            raise Fail(f'{group}: uncommitted changes; commit or stash them first')
        if not git(repo, 'rev-list', f'{base}..HEAD'):
            continue
        prefix = '' if group == ROOT_GROUP else group + '/'
        nested = {s[len(prefix):] for s in submodules if s.startswith(prefix)}
        touched = git(repo, 'diff', '--ignore-submodules=none', '--name-only', base, 'HEAD').splitlines()
        moved = sorted(nested.intersection(touched))
        if moved:
            raise Fail(f'commits in {group} move submodule pointer(s): {", ".join(moved)}\n'
                       'commit inside the submodule instead and drop the pointer change from these commits')
        changed[group] = base

    staging = HERE / 'patches.new'
    shutil.rmtree(staging, ignore_errors=True)
    for group, base in sorted(changed.items()):
        out = staging / group
        out.mkdir(parents=True)
        git(repo_path(tdesktop, group), 'format-patch', *FORMAT_PATCH_ARGS, '-o', str(out), f'{base}..HEAD')
    shutil.rmtree(PATCHES, ignore_errors=True)
    if changed:
        staging.rename(PATCHES)
    write_text(UPSTREAM_FILE, state['tag'] + '\n')
    state['heads'] = current_heads(tdesktop)
    save_state(tdesktop, state)

    for group, patches in patch_groups().items():
        print(f'{group}: {len(patches)} patch(es)')
    print(f'UPSTREAM = {state["tag"]}; commit patches/ and UPSTREAM in {HERE}')


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--tdesktop', type=Path, default=HERE.parent / 'tdesktop',
                        help='tdesktop checkout (default: %(default)s)')
    commands = parser.add_subparsers(dest='command', required=True)
    checkout = commands.add_parser('checkout', help='move tdesktop to a release and apply the patches')
    checkout.add_argument('tag', nargs='?', help='release tag (default: UPSTREAM)')
    checkout.add_argument('--force', action='store_true', help='discard uncommitted and unexported work')
    commands.add_parser('continue', help='resume applying after a resolved conflict')
    commands.add_parser('export', help='write commits back into patches/ and update UPSTREAM')
    commands.add_parser('apply', help='apply the patches to a fresh clone on the right tag (CI)')
    args = parser.parse_args()
    args.tdesktop = args.tdesktop.resolve()
    if not is_repo(args.tdesktop):
        parser.error(f'{args.tdesktop} is not a tdesktop checkout')
    handlers = {'checkout': cmd_checkout, 'continue': cmd_continue, 'export': cmd_export, 'apply': cmd_apply}
    try:
        handlers[args.command](args)
    except Fail as error:
        print(f'tele: {error}', file=sys.stderr)
        sys.exit(1)


if __name__ == '__main__':
    main()
