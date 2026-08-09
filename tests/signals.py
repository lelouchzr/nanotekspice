#!/usr/bin/env python3
"""SIGINT stops loop, restores a prompt and leaves the simulator usable."""
from pathlib import Path
import selectors
import signal
import subprocess
import sys
import tempfile
import time

binary = Path(sys.argv[1]).resolve()
with tempfile.TemporaryDirectory(prefix='nanotekspice-signal-') as directory:
    circuit = Path(directory) / 'clock.nts'
    circuit.write_text('.chipsets:\nclock c\noutput out\n.links:\nc:1 out:1\n')
    process = subprocess.Popen([str(binary), str(circuit)], stdin=subprocess.PIPE,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=directory)
    try:
        process.stdin.write(b'c=0\nloop\n')
        process.stdin.flush()
        ready = selectors.DefaultSelector()
        ready.register(process.stdout, selectors.EVENT_READ)
        received = b''
        deadline = time.monotonic() + 8
        while b'tick: 2' not in received:
            assert time.monotonic() < deadline, received
            if ready.select(.2):
                received += process.stdout.read1(65536)
        assert received.count(b'> ') == 2, 'loop printed an extra prompt'
        process.send_signal(signal.SIGINT)
        time.sleep(.05)
        process.stdin.write(b'simulate\ndisplay\nexit\n')
        process.stdin.flush()
        tail, error = process.communicate(timeout=8)
        assert process.returncode == 0 and not error, (process.returncode, error)
        assert b'> ' in tail and b'tick:' in tail, tail
        print('SIGINT loop interruption, prompt restoration and subsequent simulation passed')
    finally:
        if process.poll() is None:
            process.kill()
            process.communicate()
