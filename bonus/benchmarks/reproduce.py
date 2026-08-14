#!/usr/bin/env python3
"""Rebuild the unoptimized net resolver in an isolated copy without changing the checkout."""
import argparse
from pathlib import Path
import shutil
import subprocess
import tempfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
root = Path(__file__).resolve().parents[2]
baseline = root / 'doc/performance/baseline-source'
with tempfile.TemporaryDirectory(prefix='nanotekspice-perf-baseline-') as directory:
    work = Path(directory)
    def ignored(folder, names):
        relative = Path(folder).relative_to(root)
        return [name for name in names if name == '.git' or name.startswith('build') or
                (relative == Path('.') and name == 'nanotekspice')]
    shutil.copytree(root, work / 'source', ignore=ignored)
    for source in baseline.rglob('*'):
        if source.is_file():
            target = work / 'source' / source.relative_to(baseline)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
    build = work / 'build'
    subprocess.run(['cmake', '-S', str(work / 'source'), '-B', str(build),
                    '-DCMAKE_BUILD_TYPE=Release', '-DBUILD_TESTING=OFF',
                    '-DNTS_STRICT=ON', '-DNTS_BONUS=ON', '-DNTS_BENCHMARKS=ON'], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'nts_benchmark', '--parallel', '4'], check=True)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(build / 'bin/nts_benchmark', args.output)
print(args.output)
