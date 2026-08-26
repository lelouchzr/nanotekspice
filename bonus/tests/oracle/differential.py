#!/usr/bin/env python3
"""Compare the real CPU with an independent Python integer/reference interpreter."""
import collections
import random
import subprocess
import sys

rng = random.Random(400)
cases = []

def reference(memory, entry, budget, inputs):
    memory = dict(memory)
    queue = collections.deque(inputs)
    output = []
    pc, steps, state = entry, 0, 'ready'
    for _ in range(budget):
        if state == 'halted':
            break
        a, b, c = (memory.get(pc + i, 0) for i in range(3))
        if a == -1:
            if not queue:
                state = 'waiting'
                break
            memory[b] = queue.popleft()
            pc += 3
        elif b in (-1, -2):
            output.append((memory.get(a, 0) % 256,
                           memory.get(c, 0) % 4096 if b == -2 else len(output) % 4096,
                           int(b == -1)))
            pc += 3
        else:
            memory[b] = memory.get(b, 0) - memory.get(a, 0)
            pc = c if memory[b] <= 0 else pc + 3
        steps += 1
        state = 'halted' if pc < 0 else 'ready'
    return pc, state, steps, output, memory

for index in range(240):
    # Include addresses and arithmetic far outside every native machine word width.
    base = 10 ** 70 if index % 3 == 0 else 0
    data = 10 ** 100 if index % 3 == 0 else 1000
    memory = {data + i: rng.randrange(-10 ** 90, 10 ** 90) for i in range(12)}
    for instruction in range(12):
        address = base + instruction * 3
        kind = rng.randrange(5)
        a = -1 if kind == 0 else data + rng.randrange(12)
        b = (-1 if kind == 1 else -2) if kind in (1, 2) else data + rng.randrange(12)
        c = rng.choice([-1] + [base + n * 3 for n in range(12)])
        if b == -2:
            c = data + rng.randrange(12)
        memory.update({address: a, address + 1: b, address + 2: c})
    memory[base + 36], memory[base + 37], memory[base + 38] = data, data, -1
    inputs = [rng.randrange(256) for _ in range(rng.randrange(8))]
    cases.append((memory, base, 200, inputs))
# A self-modifying program changes the next instruction's operand before fetching it.
for value in (-255, -1, 0, 1, 257, 10 ** 120):
    memory = {0: 100, 1: 3, 2: 3, 3: 101, 4: -1, 5: 6,
              6: 102, 7: 102, 8: -1, 100: 1, 101: 0, 102: value}
    cases.append((memory, 0, 20, []))

lines = [str(len(cases))]
expected = []
for memory, entry, budget, inputs in cases:
    watches = sorted(set(memory) | {entry + 999, 10 ** 150})
    lines.append(f'{len(memory)} {entry} {budget} {len(inputs)} {len(watches)}')
    lines.extend(f'{address} {value}' for address, value in memory.items())
    lines.append(' '.join(map(str, inputs)))
    lines.append(' '.join(map(str, watches)))
    expected.append((reference(memory, entry, budget, inputs), watches))
result = subprocess.run([sys.argv[1]], input='\n'.join(lines)+'\n', text=True,
                        capture_output=True, timeout=30)
assert result.returncode == 0, result.stderr
actual = iter(result.stdout.splitlines())
for index, ((pc, state, steps, outputs, memory), watches) in enumerate(expected):
    header = next(actual).split()
    assert header == [str(pc), state, str(steps), str(len(outputs))], (index, header, pc, state, steps)
    for event in outputs:
        assert tuple(map(int, next(actual).split())) == event, (index, event)
    for address in watches:
        value = int(next(actual))
        assert value == memory.get(address, 0), (index, address, value, memory.get(address, 0))
assert next(actual, None) is None
print(f'{len(cases)} independent CPU oracle cases: exact PC, state, instruction count, memory and output bytes passed')
