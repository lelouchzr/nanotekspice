"""Exercise the actual RAM/ROM bus wiring and binary logger side effects."""
from support import Session, bus


def check(binary, root):
    coverage = {}
    image = bytes(((i * 37) ^ (i >> 3) ^ 0xA5) & 255 for i in range(2048))
    rom = Session(binary, root, 'nts_single/2716_rom.nts', files={'rom.bin': image})
    for address, value in enumerate(image):
        rom.take(bus('addr_', address, 11, digits=2) | {'enable': 0, 'read': 0}, bus('out_', value, 8))
    for controls in ({'enable': 1}, {'enable': 0, 'read': 1}, {'enable': 'U', 'read': 0}):
        rom.take(controls, {f'out_{i}': 'U' for i in range(8)})
    coverage[rom.relative] = rom.check()
    missing = Session(binary, root, rom.relative)
    missing.take(bus('addr_', 1024, 11, digits=2) | {'enable': 0, 'read': 0},
                 {f'out_{i}': 'U' for i in range(8)})
    coverage[rom.relative] += missing.check()

    ram = Session(binary, root, 'nts_single/4801_ram.nts')
    values = [((i * 17) ^ (i >> 2) ^ 0x5A) & 255 for i in range(1024)]
    for address, value in enumerate(values):
        ram.take(bus('addr_', address, 10) | bus('in_', value, 8) |
                 {'enable': 0, 'write': 0, 'read': 1}, bus('out_', value, 8))
    for address, value in enumerate(values):
        ram.take(bus('addr_', address, 10) | bus('in_', value, 8) |
                 {'write': 1, 'read': 0}, bus('out_', value, 8))
        # The fixture keeps input drivers connected to the bidirectional data pins.
        # Matching data proves known storage; opposing data must expose bus contention.
        ram.take(bus('in_', value ^ 255, 8), {f'out_{i}': 'U' for i in range(8)})
    ram.take({'enable': 1}, bus('out_', values[-1] ^ 255, 8))
    coverage[ram.relative] = ram.check()

    logger = Session(binary, root, 'nts_single/logger.nts', files={'log.bin': b'prefix'})
    names = [f'in_{1 << i:03}' for i in range(8)]
    for value in range(256):
        data = dict(zip(names, (str((value >> i) & 1) for i in range(8))))
        logger.take(data | {'clock': 0, 'inhibit': 0}, {})
        logger.take({'clock': 1}, {})
        logger.take({'clock': 1}, {})  # No second write without a new rising edge.
    logger.take({'clock': 0, 'inhibit': 1}, {})
    logger.take({'clock': 1}, {})
    logger.take({'clock': 0, 'inhibit': 0, 'in_001': 'U'}, {})
    logger.take({'clock': 1}, {})
    coverage[logger.relative] = logger.check({'log.bin': b'prefix' + bytes(range(256))})
    return coverage
