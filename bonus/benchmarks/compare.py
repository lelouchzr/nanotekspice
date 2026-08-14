#!/usr/bin/env python3
"""Run alternating paired benchmarks and require identical output digests."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--before', type=Path, required=True)
parser.add_argument('--after', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--pairs', type=int, default=7)
args = parser.parse_args()
if not 3 <= args.pairs <= 30:
    raise SystemExit('--pairs must be 3..30')
args.output.mkdir(parents=True, exist_ok=True)
cases = [('fanout', 1024, 128), ('cascade', 128, 65536), ('counters', 128, 2048)]
rows = []
for case, size, count in cases:
    for label, binary in [('before', args.before), ('after', args.after)]:
        subprocess.run([str(binary.resolve()), case, str(size), '8'], capture_output=True, check=True)
    for pair in range(args.pairs):
        versions = [('before', args.before), ('after', args.after)]
        if pair % 2:
            versions.reverse()
        digests = []
        for label, binary in versions:
            result = subprocess.run([str(binary.resolve()), case, str(size), str(count)],
                                    capture_output=True, text=True, check=True, timeout=90)
            row = dict(json.loads(result.stdout), version=label, pair=pair)
            rows.append(row)
            digests.append(row['digest'])
        if digests[0] != digests[1]:
            raise RuntimeError(f'{case}: output digest changed in pair {pair}')
summary = {}
for case, _, _ in cases:
    before = [r['seconds'] for r in rows if r['case'] == case and r['version'] == 'before']
    after = [r['seconds'] for r in rows if r['case'] == case and r['version'] == 'after']
    first, last = statistics.median(before), statistics.median(after)
    summary[case] = {'before_median_s': first, 'after_median_s': last,
                     'change_percent': (last / first - 1) * 100,
                     'all_digests_equal': True, 'pairs': args.pairs}
(args.output / 'ab.jsonl').write_text(''.join(json.dumps(row) + '\n' for row in rows))
(args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
print(json.dumps(summary, indent=2))
