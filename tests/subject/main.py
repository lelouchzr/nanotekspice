#!/usr/bin/env python3
"""Compare every supplied .nts circuit with independent truth/state models and subject transcripts."""
from pathlib import Path
import argparse
import combinational
import memory
import sequential
import transcripts


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', type=Path)
    parser.add_argument('root', type=Path)
    parser.add_argument('--valgrind', type=Path)
    parser.add_argument('--log-directory', type=Path)
    args = parser.parse_args()
    binary, root = args.binary.resolve(), args.root.resolve()
    if args.valgrind:
        if not args.log_directory:
            parser.error('--valgrind requires --log-directory')
        args.log_directory.mkdir(parents=True, exist_ok=True)
        binary = (str(args.valgrind), '--tool=memcheck', '--leak-check=full',
                  '--show-leak-kinds=all', '--errors-for-leak-kinds=all',
                  '--default-suppressions=no', '--error-exitcode=99',
                  '--log-file=' + str(args.log_directory.resolve() / 'subject-%p.log'), str(binary))
    coverage = {}
    for module in (combinational, memory, sequential):
        coverage.update(module.check(binary, root))
    expected = {str(path.relative_to(root / 'examples'))
                for directory in ('nts_single', 'nts_advanced')
                for path in (root / 'examples' / directory).glob('*.nts')}
    assert set(coverage) == expected and len(expected) == 27, (set(coverage) ^ expected)
    samples = transcripts.check(binary, root)
    for name, frames in sorted(coverage.items()):
        print(f'{name}: {frames} expected output frames matched')
    print(f'{len(coverage)} subject circuits, {sum(coverage.values())} output frames, '
          f'{samples} supplied transcripts: all comparisons passed')


if __name__ == '__main__':
    main()
