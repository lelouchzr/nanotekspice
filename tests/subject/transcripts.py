"""Replay the three supplied shell examples, including deferred assignment and EOF."""
from pathlib import Path
import re
import subprocess
import tempfile
from support import command


def check(binary, root):
    for index in (1, 2, 3):
        text = (root / 'examples/subject_sample' / f'example_{index}.txt').read_text()
        circuit_text = text.split('\n', 1)[1].split('~/B-OOP-400> ./nanotekspice', 1)[0]
        circuit_text = '\n'.join(line.removesuffix('$') for line in circuit_text.splitlines()) + '\n'
        session = text.split('~/B-OOP-400> ./nanotekspice', 1)[1].split('\n', 1)[1]
        session = session.split('~/B-OOP-400> echo $?', 1)[0]
        commands, expected = [], []
        for line in session.splitlines():
            match = re.fullmatch(r'> \*(.+)\*', line)
            if match:
                expected.append('> ')
                if match[1] != '(CTRL+D)':
                    commands.append(match[1])
            else:
                expected.append(line + '\n')
        with tempfile.TemporaryDirectory(prefix='nts-transcript-') as directory:
            circuit = Path(directory) / 'example.nts'
            circuit.write_text(circuit_text)
            result = subprocess.run(command(binary, circuit), cwd=directory,
                                    input='\n'.join(commands) + ('\n' if commands else ''),
                                    capture_output=True, text=True, timeout=15)
            if index == 2:
                # The sample's diagnostic contains a typo; check its meaning and exit status.
                assert result.returncode == 84 and not result.stdout
                assert "'a'" in result.stderr and 'unknown component' in result.stderr.lower()
            else:
                assert result.returncode == 0 and not result.stderr, result.stderr
                assert result.stdout == ''.join(expected), (index, result.stdout, ''.join(expected))
    return 3
