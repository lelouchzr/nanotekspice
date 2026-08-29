# Runnable circuits

All circuit descriptions are committed as `.nts` source files. Run commands from the repository root.

| Folder | Contents |
|---|---|
| [logic/](logic/) | Half adder and subject OR-gate example |
| [chips/](chips/) | Sources, basic gates, CMOS packages, counters, RAM, ROM and logger |
| [advanced/](advanced/) | Combined logic and counter feedback |
| [Bonus circuits](../bonus/examples/) | Seven-segment display, matrix, CPU computer and keyboard bus; SUBLEQ programs included |

```sh
./nanotekspice examples/logic/half_adder.nts
```

Enter `a=1`, `b=1`, `simulate`, then `display`: `sum: 0`, `carry: 1`.
Assignments take effect at the next simulation tick. Inputs start at U.

```sh
./nanotekspice --set in_reset=0 --set cl_clock=0 --ticks 8 examples/chips/4040_counter.nts
./nanotekspice --program cpu bonus/examples/paint.subleq --set clk=0 --gui bonus/examples/computer.nts
```

The `--set`, `--ticks`, and GUI options require bonuses. The interactive shell works in core builds.
The ROM reads `rom.bin` and the logger writes `log.bin` in the current working directory.
A missing ROM preserves unknown values. Generated binaries, logs and exports are not part of the source catalog.

Chip and combined-circuit examples are adapted from the supplied project fixtures. Their reference copies remain in `tests/fixtures/original/`; see [third-party notices](../THIRD_PARTY_NOTICES.md).
