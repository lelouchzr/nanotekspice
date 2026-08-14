#!/usr/bin/env python3
"""Collect native macOS sample stacks while the real simulator runs deterministic workloads."""
import argparse
import json
from pathlib import Path
import platform
import subprocess
import time

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--binary', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
if platform.system() != 'Darwin':
    raise SystemExit('This collector requires macOS sample; timings can run on all platforms.')
args.output.mkdir(parents=True, exist_ok=True)
metadata = []
for case, size, count in [('fanout', 1024, 128), ('cascade', 128, 8192), ('counters', 128, 2048)]:
    process = subprocess.Popen([str(args.binary.resolve()), case, str(size), str(count), '4'],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        time.sleep(.2)
        sample = subprocess.run(['sample', str(process.pid), '2', '1', '-file',
                                 str(args.output / f'{case}.sample.txt')],
                                capture_output=True, text=True, timeout=15)
        output, error = process.communicate(timeout=15)
        if sample.returncode or process.returncode:
            raise RuntimeError((sample.returncode, sample.stderr, process.returncode, error))
        metadata.append(json.loads(output))
    finally:
        if process.poll() is None:
            process.kill()
            process.communicate()
(args.output / 'runs.json').write_text(json.dumps(metadata, indent=2) + '\n')
print(json.dumps(metadata))
