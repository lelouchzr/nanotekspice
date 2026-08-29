#!/usr/bin/env python3
"""Load every published circuit and check the half-adder truth table through the real shell."""
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    binary, root = (Path(value).resolve() for value in sys.argv[1:3])
    bonus = sys.argv[3] == '1'
    circuits = sorted((root / 'examples').rglob('*.nts'))
    if bonus:
        circuits += sorted((root / 'bonus/examples').glob('*.nts'))
    assert circuits, 'The public circuit catalog is empty'
    with tempfile.TemporaryDirectory(prefix='nts-examples-') as directory:
        for circuit in circuits:
            result = subprocess.run([str(binary), str(circuit)], cwd=directory,
                                    input='display\nsimulate\ndisplay\nexit\n',
                                    capture_output=True, text=True, timeout=15)
            assert result.returncode == 0, (circuit.name, result.stderr)
            assert not result.stderr and 'tick: 0' in result.stdout and 'tick: 1' in result.stdout
        commands = ''.join(f'a={a}\nb={b}\nsimulate\ndisplay\n'
                           for a, b in ((0, 0), (0, 1), (1, 0), (1, 1))) + 'exit\n'
        result = subprocess.run([str(binary), str(root / 'examples/logic/half_adder.nts')],
                                cwd=directory, input=commands, text=True,
                                capture_output=True, timeout=15)
        assert result.returncode == 0 and not result.stderr, result.stderr
        for tick, carry, total in ((1, 0, 0), (2, 0, 1), (3, 0, 1), (4, 1, 0)):
            display = result.stdout.split(f'tick: {tick}\n', 1)[1].split('> ', 1)[0]
            assert f'  carry: {carry}\n' in display and f'  sum: {total}\n' in display, display
    print(f'{len(circuits)} published circuits loaded; all four half-adder results correct')


if __name__ == '__main__':
    main()
