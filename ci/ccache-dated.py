#!/usr/bin/env python3
"""compile the objects whose sources use __DATE__, __TIME__ or __TIMESTAMP__ again, past ccache.

precompiled headers need ccache's time_macros sloppiness, so a cache hit would carry the date of the build that
filled the cache. run after a full build: the objects are deleted and compiled with CCACHE_RECACHE set, and the
target links again."""

import json
import os
import re
import subprocess
import sys

MACRO = re.compile(rb'\b__(DATE|TIME|TIMESTAMP)__\b')
OBJECT = ('.obj', '.o')


def main():
    if len(sys.argv) != 4:
        sys.exit(f'usage: {sys.argv[0]} BUILD_DIR CONFIG TARGET')
    build, config, target = sys.argv[1:]
    with open(os.path.join(build, 'CMakeCache.txt'), encoding='utf-8') as file:
        found = re.search(r'^CMAKE_MAKE_PROGRAM:[A-Z]+=(.+)$', file.read(), re.M)
    if not found:
        sys.exit('no CMAKE_MAKE_PROGRAM in CMakeCache.txt')
    ninja = found.group(1).strip()
    compdb = json.loads(subprocess.run(
        [ninja, '-C', build, '-f', f'build-{config}.ninja', '-t', 'compdb'],
        check=True, capture_output=True, text=True).stdout)

    dated = []
    for entry in compdb:
        if 'output' not in entry:
            sys.exit(f'{ninja} -t compdb has no output field, ninja is too old')
        if not entry['output'].endswith(OBJECT):
            continue
        source = os.path.join(entry['directory'], entry['file'])
        try:
            with open(source, 'rb') as file:
                text = file.read()
        except OSError:
            continue
        if MACRO.search(text):
            dated.append((entry['file'], os.path.join(entry['directory'], entry['output'])))

    if not dated:
        print('no source uses __DATE__, __TIME__ or __TIMESTAMP__')
        return
    for source, output in dated:
        print(f'compiling again: {source}')
        if os.path.exists(output):
            os.remove(output)
    subprocess.run(
        ['cmake', '--build', build, '--config', config, '--parallel', '--target', target],
        check=True, env={**os.environ, 'CCACHE_RECACHE': '1'})


if __name__ == '__main__':
    main()
