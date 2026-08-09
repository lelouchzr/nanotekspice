#!/usr/bin/env python3
"""Exercise the real executable, subject protocol and all original circuit fixtures."""
import os
from pathlib import Path
import subprocess
import sys
import tempfile

binary = Path(sys.argv[1]).resolve()
root = Path(sys.argv[2]).resolve()
bonus = sys.argv[3] == '1'
checks = 0


def run(args, text='', code=0, work=None):
    global checks
    result = subprocess.run([str(binary), *map(str, args)], input=text,
                            capture_output=True, text=True, cwd=work, timeout=12)
    checks += 1
    assert result.returncode == code, (args, result.returncode, result.stdout, result.stderr)
    if code:
        assert result.stderr and not result.stdout, (args, result.stdout, result.stderr)
    return result


with tempfile.TemporaryDirectory(prefix='nanotekspice-cli-') as directory:
    work = Path(directory)
    circuit = work / 'space and accent é.nts'
    circuit.write_text('.chipsets:\nclock cl\noutput out\n.links:\nout:1 cl:1\n')
    result = run([circuit], 'display\ncl=0\ndisplay\nsimulate\ndisplay\nsimulate\n'
                 'display\nsimulate\nsimulate\nsimulate\ndisplay\nexit\n', work=work)
    assert result.stdout.count('tick: 0\ninput(s):\n  cl: U\noutput(s):\n  out: U') == 2
    for tick, value in [(1, 0), (2, 1), (5, 0)]:
        assert f'tick: {tick}\ninput(s):\n  cl: {value}\noutput(s):\n  out: {value}' in result.stdout
    assert not result.stderr
    run([], code=84, work=work)
    run([work / 'missing.nts'], code=84, work=work)
    run([work], code=84, work=work)
    assert run(['--help'], work=work).stdout.startswith('USAGE:')
    read_fd, write_fd = os.pipe()
    os.close(read_fd)
    try:
        broken_stdout = subprocess.run([str(binary), '--help'], stdout=write_fd,
                                       stderr=subprocess.PIPE, cwd=work, timeout=12)
    finally:
        os.close(write_fd)
    assert broken_stdout.returncode == 84 and broken_stdout.stderr
    checks += 1
    assert run([circuit], work=work).stdout == '> '
    assert run([circuit], '\n \t\nexit', work=work).stdout == '> > > '
    for broken in ['bad', '.chipsets:\n.links:\n', '.chipsets:\ninput a\n.links:\na:1 b:1\n',
                   '.chipsets:\nnot a\n.links:\na:20 a:1\n']:
        circuit.write_text(broken)
        run([circuit], code=84, work=work)
    for fixture in sorted((root / 'tests/fixtures/original').rglob('*.nts')):
        # This original scratch file intentionally declares a nonexistent component.
        source = fixture.read_text()
        if 'this_component_does_not_exist' in source:
            continue
        result = subprocess.run([str(binary), str(fixture)], input='display\nsimulate\ndisplay\nexit\n',
                                capture_output=True, text=True, cwd=work, timeout=12)
        checks += 1
        assert result.returncode == 0, (fixture, result.stderr)
        assert 'tick: 0' in result.stdout and 'tick: 1' in result.stdout, fixture
    circuit.write_text('.chipsets:\ninput a\noutput o\n.links:\na:1 o:1\n')
    if bonus:
        svg, html = work / 'circuit.svg', work / 'circuit.html'
        run(['--set', 'a=1', '--ticks', '1', '--export-svg', svg, '--export-html', html, circuit], work=work)
        assert svg.exists() and '<svg' in svg.read_text()
        assert html.exists() and '<html' in html.read_text().lower()
        for args in [['--ticks', '-1', circuit], ['--ticks', '999999999999999999999999', circuit],
                     ['--set', 'simulate', circuit], ['--gui', '--program'], ['--program', 'a', 'missing', circuit]]:
            run(args, code=84, work=work)
    else:
        run(['--gui', circuit], code=84, work=work)
print(f'{checks} real CLI scenarios passed')
