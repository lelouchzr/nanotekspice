# NanoTekSpice requirements

Reference: [subject v3.1.2](subject.pdf). [Pinouts](COMPONENTS.md).

| Requirement | Implementation | Evidence |
|---|---|---|
| C++, nanotekspice executable, Make/CMake, clean/fclean/re | C++20, CMake, Make wrapper, selected-variant staging | Strict macOS/Linux builds, ON/OFF |
| stderr diagnostics, exit 84/0 | std::exception hierarchy, entry-point catches, stream checks | CLI usage/missing file/syntax/closed stdout, ROM/logger errors |
| U and complete truth tables | Tristate, ternary logic, internal high impedance | All nine 0/1/U pairs per gate |
| Graph from .nts | Declarations, unique names, two sections, inline/full comments, whitespace | `tests/parser.cpp`, `tests/cli.py`, 4,000 pseudorandom inputs |
| Bidirectional links/fanout | Shared pin nets, chained inputs, multiple drivers, conflicts→U | `tests/basic/BasicConnections.cpp`: merge/conflict/destruction/cache |
| Lexical errors, unknown types/names, duplicates, empty circuit | Bounded parser, factory, pin checks | Parser/CLI, truncated lines, NUL, out-of-range integers |
| IComponent and abstract graph ownership | Required interface, Node owns unique_ptr<IComponent>, subject factory signature | Header compilation, Circuit/Factory review; no concrete graph references |
| Inputs/clocks/constants/outputs | Initial U, deferred assignments, clock inversion after assignment | Exact tick0/tick1/tick2/tick5 subject example, sources/CLI |
| and/or/xor/not and six DIP gates | Datasheet pinouts, independent gates | BasicTruth, every gate/output |
| 4008 | Ternary full adder | All 512 binary combinations and U |
| 4013 | Dual D flip-flop, asynchronous set/reset, complementary outputs | DualFlipFlop, both cascade orders, simultaneous controls, U |
| 4017 | Johnson counter with both clock-edge modes | Full cycle, carry/reset/inhibit |
| 4040 | Falling-edge 12-bit counter | All 4,096 states and reset |
| 4094 | Shift, strobe latch, parallel OE, opposite-edge serial outputs | Shift tests: edges/latch/OE/U |
| 4512 | Eight-channel selector, inhibit/OE per datasheet | All data/addresses, high impedance/U |
| 4514 | Address latch, 4→16 decoder | Addresses/latch/inhibit/U |
| 4801 | 1024×8 RAM, shared bus, active-low controls | All addresses, varying high bits, reads/writes/ambiguity |
| 2716 | 2048×8 ROM from ./rom.bin | All addresses, short/absent/invalid files, A10 pin 19 |
| Logger | Exact appended byte on rising edge, inhibit false | NUL/high bits/U/inhibit/file errors |
| ASCII-sorted display and '> ' prompt | Shell and ordered names | Actual protocol/output order |
| Deterministic sequential simulation | Global beginTick/prepare/simulate/endTick passes | Order-independent cascades |
| loop until SIGINT without prompt | Minimal sig_atomic_t handler, previous handler restored | Real subprocess: SIGINT, prompt, further simulation |
| Clean EOF/exit | No deep exit() or leaked components | CLI, immediate EOF, final line without newline |
| No accidental unbounded recursion/loops | Size/cycle/depth/I/O guards | ASan/UBSan, edge cases, deterministic fuzz, errors |
| Bonus files under bonus/ | Sources/programs/tests/captures/docs | ON/OFF builds, repository inventory |

[Five bonuses](../bonus/README.md) have dedicated tests. SUBLEQ has [explicit budgets](../bonus/CPU.md). Pin-level tests supplement integrated shell/computer/GUI scenarios. Digital simulation only, without analog delays.
