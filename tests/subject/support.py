"""Black-box shell sessions: expected values are supplied independently of the C++ engine."""
from pathlib import Path
import re
import subprocess
import tempfile


def bus(prefix, value, width, digits=1, first=0):
    return {f'{prefix}{index + first:0{digits}d}': str((value >> index) & 1)
            for index in range(width)}


def logic(kind, *values):
    if kind == 'not':
        return 'U' if values[0] == 'U' else str(1 - int(values[0]))
    if kind in ('and', 'nand'):
        result = '0' if '0' in values else 'U' if 'U' in values else '1'
    elif kind in ('or', 'nor'):
        result = '1' if '1' in values else 'U' if 'U' in values else '0'
    else:
        result = 'U' if 'U' in values else str(sum(map(int, values)) % 2)
    return logic('not', result) if kind in ('nand', 'nor') else result


def command(binary, circuit):
    prefix = list(binary) if isinstance(binary, tuple) else [str(binary)]
    return prefix + [str(circuit)]


class Session:
    def __init__(self, binary, root, relative, files=None, source=None):
        self.binary = binary
        self.relative = relative
        self.circuit = root / 'examples' / relative
        self.source = source
        declarations = (source if source is not None else self.circuit.read_text()).split('.links:')[0]
        self.clocks = set(re.findall(r'^\s*clock\s+(\S+)', declarations, re.M))
        self.values = {name: 'U' for _, name in re.findall(
            r'^\s*(input|clock)\s+(\S+)', declarations, re.M)}
        self.outputs = set(re.findall(r'^\s*output\s+(\S+)', declarations, re.M))
        self.tick = 0
        self.commands = []
        self.expected = []
        self.files = files or {}

    def take(self, assignments, outputs, *, advance=True):
        assert set(assignments) <= self.values.keys(), (self.relative, assignments)
        assert set(outputs) == self.outputs, (self.relative, set(outputs) ^ self.outputs)
        self.commands.extend(f'{name}={value}' for name, value in assignments.items())
        if advance:
            for name in self.clocks - assignments.keys():
                if self.values[name] != 'U':
                    self.values[name] = str(1 - int(self.values[name]))
            self.values.update({name: str(value) for name, value in assignments.items()})
            self.commands.append('simulate')
            self.tick += 1
        else:
            assert not assignments, 'Use transcript checks for pending input assignments'
        self.commands.append('display')
        self.expected.append((self.tick, dict(self.values), {k: str(v) for k, v in outputs.items()}))

    def check(self, expected_files=None):
        with tempfile.TemporaryDirectory(prefix='nts-subject-') as directory:
            work = Path(directory)
            for name, data in self.files.items():
                (work / name).write_bytes(data)
            circuit = self.circuit
            if self.source is not None:
                circuit = work / 'circuit.nts'
                circuit.write_text(self.source)
            result = subprocess.run(command(self.binary, circuit), cwd=work,
                                    input='\n'.join(self.commands + ['exit']) + '\n',
                                    text=True, capture_output=True, timeout=60)
            assert result.returncode == 0 and not result.stderr, (self.relative, result.stderr)
            frames = re.findall(r'tick: (\d+)\ninput\(s\):\n(.*?)output\(s\):\n(.*?)(?=> |\Z)',
                                result.stdout, re.S)
            assert len(frames) == len(self.expected), (self.relative, len(frames), len(self.expected))
            for frame, expected in zip(frames, self.expected):
                actual = (int(frame[0]), dict(re.findall(r'^  (\S+): ([01U])$', frame[1], re.M)),
                          dict(re.findall(r'^  (\S+): ([01U])$', frame[2], re.M)))
                assert actual == expected, (self.relative, 'actual', actual, 'expected', expected)
            for name, expected in (expected_files or {}).items():
                assert (work / name).read_bytes() == expected, (self.relative, name)
        return len(frames)
