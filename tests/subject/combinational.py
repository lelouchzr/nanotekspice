"""Truth tables and arithmetic checked through the supplied circuit wiring."""
from itertools import product
from support import Session, bus, logic


def check(binary, root):
    coverage = {}

    def run(name, cases):
        session = Session(binary, root, name)
        for assignments, outputs in cases:
            session.take(assignments, outputs)
        coverage[name] = session.check()

    for kind in ('and', 'or', 'xor'):
        run(f'nts_single/{kind}.nts',
            [({'in_1': a, 'in_2': b}, {'out': logic(kind, a, b)})
             for a, b in product('01U', repeat=2)])
    run('nts_single/not.nts', [({'in': value}, {'out': logic('not', value)}) for value in '01U'])
    run('nts_single/input_output.nts', [({'in': value}, {'out': value}) for value in '01U'])
    for constant in ('true', 'false'):
        run(f'nts_single/{constant}.nts', [({}, {'out': int(constant == 'true')})] * 3)

    pairs = ((1, 2, 3), (5, 6, 4), (8, 9, 10), (12, 13, 11))
    for chip, kind in (('4001_nor', 'nor'), ('4011_nand', 'nand'), ('4030_xor', 'xor'),
                       ('4071_or', 'or'), ('4081_and', 'and')):
        cases = []
        vectors = list(product('01', repeat=8))
        vectors += [tuple((a, b)[i % 2] for i in range(8)) for a, b in product('01U', repeat=2)]
        for values in vectors:
            inputs, outputs = {}, {}
            for index, (a, b, out) in enumerate(pairs):
                inputs.update({f'in_{a:02}': values[index * 2], f'in_{b:02}': values[index * 2 + 1]})
                outputs[f'out_{out:02}'] = logic(kind, values[index * 2], values[index * 2 + 1])
            cases.append((inputs, outputs))
        run(f'nts_single/{chip}.nts', cases)

    pins = ((1, 2), (3, 4), (5, 6), (9, 8), (11, 10), (13, 12))
    run('nts_single/4069_not.nts',
        [({f'in_{a:02}': value for (a, _), value in zip(pins, values)},
          {f'out_{b:02}': logic('not', value) for (_, b), value in zip(pins, values)})
         for values in product('01U', repeat=6)])
    run('nts_advanced/and.nts',
        [(dict(zip(('in_a', 'in_b', 'in_c', 'in_d', 'in_e'), values)),
          {'out': logic('and', *values)}) for values in product('01U', repeat=5)])
    run('nts_advanced/and-or-not.nts',
        [({'in_a': a, 'in_b': b, 'in_c': c},
          {'out': logic('and', logic('not', logic('or', a, logic('not', b))), c)})
         for a, b, c in product('01U', repeat=3)])

    cases = []
    for a, b, carry in product(range(16), range(16), range(2)):
        expected = a + b + carry
        cases.append((bus('in_a', a, 4, first=1) | bus('in_b', b, 4, first=1) | {'in_c': carry},
                      bus('out_', expected, 4) | {'out_c': (expected >> 4) & 1}))
    run('nts_single/4008_adder.nts', cases)

    cases = []
    for data, address in product(range(256), range(8)):
        cases.append((bus('in_', data, 8) | dict(zip(('in_a', 'in_b', 'in_c'),
                                                   (str((address >> i) & 1) for i in range(3)))) |
                      {'enable': 0, 'inhibit': 0}, {'out_data': (data >> address) & 1}))
    cases.extend([({'enable': 1}, {'out_data': 'U'}),
                  ({'enable': 0, 'inhibit': 1}, {'out_data': 0}),
                  (bus('in_', 255, 8) | {'inhibit': 0, 'in_a': 'U'}, {'out_data': 1})])
    run('nts_single/4512_selector.nts', cases)

    cases = []
    for value in range(16):
        cases.append((bus('in_', value, 4) | {'strobe': 1, 'inhibit': 0},
                      {f'out_{i:02}': int(i == value) for i in range(16)}))
    cases.append((bus('in_', 0, 4) | {'strobe': 0}, {f'out_{i:02}': int(i == 15) for i in range(16)}))
    cases.append(({'inhibit': 1}, {f'out_{i:02}': 0 for i in range(16)}))
    for bits in product('01U', repeat=4):
        candidates = [i for i in range(16) if all(bit == 'U' or int(bit) == ((i >> k) & 1)
                                                for k, bit in enumerate(bits))]
        cases.append((dict(zip((f'in_{i}' for i in range(4)), bits)) | {'strobe': 1, 'inhibit': 0},
                      {f'out_{i:02}': 'U' if i in candidates and len(candidates) > 1 else int(i in candidates)
                       for i in range(16)}))
    run('nts_single/4514_decoder.nts', cases)
    return coverage
