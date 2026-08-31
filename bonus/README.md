# NanoTekSpice bonuses

All five bonuses live in `bonus/`, enabled by `NTS_BONUS=ON`. The native window also requires SDL2 and `NTS_BUILD_GRAPHICS=ON`; components/exports work headlessly.

| Subject proposal | Implementation | Evidence |
|---|---|---|
| Circuit graph display | Native SDL2 graph, zoom/pan, pins, simulation, input editing; standalone SVG/HTML | `tests/GraphTests.cpp`, `CircuitGraph`, native smoke, CLI integration |
| Four-input `7seg` | Actual hexadecimal 0..F segments, explicit U | `tests/DisplayTests.cpp`: sixteen values, bit order, U, invalid pins |
| `matrix`, eight color/twelve address bits | 64×64 RGB332 memory, X6/Y6 addressing, unknown pixels | `tests/DisplayTests.cpp`: all 4,096 addresses, colors, unknown inputs |
| i4004 or other Turing-complete processor | Variable-precision SUBLEQ, associative memory, branches, self-modifying code | `tests/IntegerTests.cpp`, `CpuTests.cpp`, [model/budgets](CPU.md) |
| Small computer | CPU wired to displays, clock, reset/enable, keyboard bus; computing/looping/drawing programs | `examples/computer.nts`, `examples/keyboard.nts`, `tests/ComputerTests.cpp` |

## Try it

```sh
./nanotekspice --program cpu bonus/examples/triangular.subleq --set clk=0 \
  --ticks 150 --export-html /tmp/computer.html bonus/examples/computer.nts
./nanotekspice --program cpu bonus/examples/paint.subleq --set clk=0 \
  --gui bonus/examples/computer.nts
```

`triangular` computes **55 (0x37)**; low bits display 7. `paint` writes **all 4,096 pixels**. Other programs: `countdown`, `hello`, `echo`. The shell accepts `load cpu "program.subleq"`.

## Native window

| Control | Effect |
|---|---|
| Enter/F10 / Step button | One real circuit tick |
| Space / Run button | Run batches of 32 ticks; repeat to pause |
| Wheel / trackpad | Smooth zoom at the pointer, 10–400%; toolbar/inspector scrolling leaves the circuit unchanged |
| `+` / `=` / keypad `+`, `-` / keypad `-` | Zoom in/out at the canvas center |
| Ctrl+0 / Cmd+0 | Reset to 100% and the original position |
| Home | Fit the complete circuit within the zoom limits |
| Drag background | Pan graph |
| Click component | Inspect pins/display |
| Click input/clock | Select 0/1/U; applied on next simulation |
| 0, 1, U with input selected | Queue assignment, same deferred behavior as shell |
| Reset view | Restore 100% zoom and the original position |
| Escape / close window | Clean shutdown |

The toolbar shows shortcuts and the current zoom percentage. Labels scale with the circuit; tiny labels are hidden below 50% to keep the overview clear. Resizing preserves the point at the canvas center.

The GUI controls the actual `Circuit`. SVG/HTML are **snapshots**, without a JavaScript simulator. Optional `.graphic:` entries use `name x y`; integer coordinates ±1,000,000, existing component names. Otherwise automatic grid placement.

## Displays

| Component | Inputs | Behavior |
|---|---|---|
| `7seg` | Pins 1..4, least significant bit first | Hex 0..F; any U bit gives an unknown display |
| `matrix` | 1..8: RGB332; 9..14: X; 15..20: Y | 64×64 pixels, address `x+64*y` |

Pixels start unknown. U address: no write; known address with U color: mark that pixel unknown. Devices sample before CPU commit, so new CPU outputs appear on the next tick.

## Check

```sh
NTS_GUI_SMOKE_FRAMES=2 NTS_GUI_CAPTURE=/tmp/computer.bmp \
./nanotekspice --program cpu bonus/examples/paint.subleq --set clk=0 \
  --ticks 42000 --gui bonus/examples/computer.nts
```

Captured from the real SDL framebuffer. [Tests and memory checks](../doc/testing.md) · [CPU design and budgets](CPU.md).
