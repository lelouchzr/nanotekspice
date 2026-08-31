#!/usr/bin/env python3
"""Check type placement, complete implementations and formatting of maintained C++ sources."""
import pathlib
import re
import subprocess
import sys

root = pathlib.Path(sys.argv[1]).resolve()
formatter = sys.argv[2]
exclusions = ()
files = sorted(p for folder in ('include', 'src', 'bonus', 'tests')
               for p in (root / folder).rglob('*') if p.suffix in ('.cpp', '.hpp')
               and not any(str(p.relative_to(root)).startswith(e) for e in exclusions))
errors = []
for path in files:
    result = subprocess.run([formatter, '--dry-run', '--Werror', str(path)], capture_output=True)
    if result.returncode:
        errors.append(f'{path.relative_to(root)}: format differs from .clang-format')
        errors.append(result.stderr.decode(errors='replace').strip())
    text = path.read_text()
    if path.suffix == '.cpp':
        if re.search(r'^\s*(?:class|struct|enum)\s+\w+(?:\s+final)?\s*(?::[^;{]+)?\s*\{', text, re.M):
            errors.append(f'{path.relative_to(root)}: type definition belongs in a header')
    if re.search(r'\b(?:TODO|FIXME|not implemented)\b', text, re.I):
        errors.append(f'{path.relative_to(root)}: unfinished marker')
if errors:
    subprocess.run([formatter, '--version'], check=False)
    print('\n'.join(errors))
    sys.exit(1)
print(f'{len(files)} C++ files: type placement and formatting passed')
