#!/usr/bin/env python3
"""Clean build and sequential inherited regressions; optional full Lab 3 grade."""

import argparse
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pgtbl', action='store_true',
                        help='also require all Page Table features to pass')
    args = parser.parse_args()
    output = ROOT / 'artifacts/verification'
    output.mkdir(parents=True, exist_ok=True)
    context = {
        'commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT,
                                          text=True).strip(),
        'working_tree_status': subprocess.check_output(
            ['git', 'status', '--porcelain'], cwd=ROOT, text=True).splitlines(),
        'runs': [],
    }
    commands = [('clean', ['make', 'clean']),
                ('build', ['make', '-j4', 'kernel/kernel', 'fs.img']),
                ('lab01', ['make', 'check-lab01']),
                ('trace', ['make', 'check-trace']),
                ('lab02', ['make', 'check-lab02'])]
    if args.pgtbl:
        commands.append(('pgtbl', ['make', 'grade']))
    for name, command in commands:
        print('$ ' + ' '.join(command), flush=True)
        with (output / (name + '.txt')).open('w') as log:
            result = subprocess.run(command, cwd=ROOT, stdout=log,
                                    stderr=subprocess.STDOUT)
        context['runs'].append({'command': command, 'returncode': result.returncode,
                                'log': name + '.txt'})
        (output / 'context.json').write_text(
            json.dumps(context, ensure_ascii=False, indent=2) + '\n')
        print(f'  exit={result.returncode}; artifacts/verification/{name}.txt', flush=True)
        if result.returncode:
            raise SystemExit(result.returncode)


if __name__ == '__main__':
    main()
