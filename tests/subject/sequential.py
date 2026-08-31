"""Independent edge/state models for the supplied sequential circuits."""
from support import Session, bus


def check(binary, root):
    coverage = {}
    clock = Session(binary, root, 'nts_single/clock.nts')
    clock.take({}, {'out': 'U'}, advance=False)
    clock.take({'cl': 0}, {'out': 0})
    for tick in range(2, 33):
        clock.take({}, {'out': int(tick % 2 == 0)})
    clock.take({'cl': 'U'}, {'out': 'U'})
    clock.take({}, {'out': 'U'})
    coverage[clock.relative] = clock.check()

    decade = Session(binary, root, 'nts_single/4017_johnson.nts')

    def johnson(state):
        return bus('out_', 1 << state, 10) | {'out_s': int(state < 5)}

    decade.take({'in_0': 0, 'in_1': 0, 'in_r': 1}, johnson(0))
    decade.take({'in_r': 0}, johnson(0))
    for count in range(1, 51):
        decade.take({'in_0': 1}, johnson(count % 10))
        decade.take({'in_0': 0}, johnson(count % 10))
    decade.take({'in_1': 1}, johnson(0))
    decade.take({'in_0': 1}, johnson(0))
    decade.take({'in_1': 0}, johnson(1))
    decade.take({'in_r': 1}, johnson(0))
    coverage[decade.relative] = decade.check()

    binary_counter = Session(binary, root, 'nts_single/4040_counter.nts')
    binary_counter.take({'cl_clock': 1, 'in_reset': 1}, bus('out_', 0, 12, digits=2))
    binary_counter.take({'cl_clock': 1, 'in_reset': 0}, bus('out_', 0, 12, digits=2))
    for value in range(1, 4097):
        binary_counter.take({'cl_clock': 0}, bus('out_', value % 4096, 12, digits=2))
        binary_counter.take({'cl_clock': 1}, bus('out_', value % 4096, 12, digits=2))
    coverage[binary_counter.relative] = binary_counter.check()

    shift = Session(binary, root, 'nts_single/4094_shift.nts')
    register, delayed = ['U'] * 8, 'U'

    def shifted():
        return dict(zip((f'out_{i}' for i in range(8)), register)) | {
            'out_qs': register[-1], 'out_qe': delayed}

    shift.take({'clock': 0, 'data': 0}, shifted())
    for value in list('1010011011000101') + ['U'] + list('0000000011111111'):
        register = [value] + register[:-1]
        shift.take({'clock': 1, 'data': value}, shifted())
        shift.take({'clock': 1}, shifted())  # A held high clock must not shift again.
        delayed = register[-1]
        shift.take({'clock': 0}, shifted())
    coverage[shift.relative] = shift.check()

    flip = Session(binary, root, 'nts_single/4013_flipflop.nts')
    q, qb = [0, 0], [1, 1]

    def flips():
        return {f'out_{i + 1}_q': q[i] for i in range(2)} | {
            f'out_{i + 1}_qb': qb[i] for i in range(2)}

    controls = {f'{prefix}_{i}_{suffix}': value for i in (1, 2)
                for prefix, suffix, value in (('cl', 'clock', 0), ('in', 'reset', 1),
                                             ('in', 'set', 0), ('in', 'data', 0))}
    flip.take(controls, flips())
    flip.take({'in_1_reset': 0, 'in_2_reset': 0}, flips())
    for first, second in ((1, 0), (0, 1), (1, 1), ('U', 0), (0, 0)):
        flip.take({'cl_1_clock': 0, 'cl_2_clock': 0, 'in_1_data': first, 'in_2_data': second}, flips())
        q = [first, second]
        qb = ['U' if v == 'U' else 1 - v for v in q]
        flip.take({'cl_1_clock': 1, 'cl_2_clock': 1}, flips())
    q, qb = [1, 1], [1, 1]
    flip.take({'in_1_set': 1, 'in_2_set': 1, 'in_1_reset': 1, 'in_2_reset': 1,
               'cl_1_clock': 1, 'cl_2_clock': 1}, flips())
    q, qb = [0, 1], [1, 0]
    flip.take({'in_1_set': 0, 'in_2_reset': 0, 'cl_1_clock': 1, 'cl_2_clock': 1}, flips())
    coverage[flip.relative] = flip.check()

    # The feedback wire resets bit four immediately, giving a modulo-16 counter.
    relative = 'nts_advanced/altered-counter.nts'
    original = (root / 'examples' / relative).read_text()
    for variant in (None, original.replace('4040 counter', '4040 z_counter')
                    .replace('counter:', 'z_counter:').replace('4071 or', '4071 a_or')
                    .replace('or:', 'a_or:')):
        counter = Session(binary, root, relative, source=variant)
        counter.take({'in_reset': 1, 'cl_clock': 1}, {'out_1': 0, 'out_2': 0})
        counter.take({'in_reset': 0, 'cl_clock': 1}, {'out_1': 0, 'out_2': 0})
        for value in range(1, 129):
            state = value % 16
            expected = {'out_1': int(bool(state & 3)), 'out_2': int(bool(state & 12))}
            counter.take({'cl_clock': 0}, expected)
            counter.take({'cl_clock': 1}, expected)
        coverage[relative] = coverage.get(relative, 0) + counter.check()
    return coverage
