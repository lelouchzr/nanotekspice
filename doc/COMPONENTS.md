# Components and pinouts

References: [subject](subject.pdf) and manufacturer manuals below. Digital links to power/unused pins are rejected.

## Semantics

| Case | Result |
|---|---|
| Unconnected input, uninitialized storage | U |
| `0 AND U` / `1 OR U` | 0 / 1 |
| Conflicting active drivers | U |
| Disabled three-state output | High impedance, ignored by resolver |
| Assign U to an input | Active unknown driver, **not** a released bus |
| Uncertain edge/control | Conservative merge of possible states |

```mermaid
flowchart LR
    Begin[beginTick: inputs and clocks] --> Prepare[prepare: sample every component]
    Prepare --> Commit[simulate: commit every register]
    Commit --> End[endTick: invalidate caches]
```

Each global pass finishes before the next begins. The same tick is not committed twice. Sequential results do not depend on component-name order.

## Sources and gates

| Type | Pins |
|---|---|
| `input`, `clock`, `true`, `false` | Output 1; deferred input/clock assignments |
| `output` | Observe 1 |
| `and`, `or`, `xor` | Inputs 1,2 → output 3 |
| `not` | 1 → 2 |
| `4001` NOR, `4011` NAND, `4030` XOR, `4071` OR, `4081` AND | (1,2)→3; (5,6)→4; (8,9)→10; (12,13)→11 |
| `4069` NOT | 1→2; 3→4; 5→6; 9→8; 11→10; 13→12 |

14-pin DIP packages use 7/14 for power. A clock applies its pending assignment on the next tick, then inverts on subsequent ticks; U remains U.

## Advanced circuits

| Type | Input pins | Output pins and behavior |
|---|---|---|
| `4008` | A0..A3: **7, 5, 3, 1**. B0..B3: **6, 4, 2, 15**. Carry-in: **9**. | Sum bits 0..3: **10, 11, 12, 13**. Carry-out: **14**. Combinational four-bit full adder. [Manual][4008]. |
| `4013` | First half: clock **3**, reset **4**, D **5**, set **6**. Second: set **8**, D **9**, reset **10**, clock **11**. | First Q / complementary Q: **1 / 2**. Second: **13 / 12**. D is captured on the positive clock edge. Set and reset are asynchronous and active high. Both asserted drive **both outputs high**, as specified by the supplied truth table. [Manual][4013]. |
| `4017` | CP0 **14**, CP1 **13**, master reset **15**. | Q0..Q9: **3, 2, 4, 7, 10, 1, 5, 6, 9, 11**. Carry **12** is high in states 0..4 and low in states 5..9. Advances on CP0 rising while CP1 is low, or CP1 falling while CP0 is high. Active-high reset selects Q0 and asserts carry. [Manual][4017]. |
| `4040` | Clock **10**, reset **11**. | Counter bits 0..11: **9, 7, 6, 5, 3, 2, 4, 13, 12, 14, 15, 1**. Counts on the **negative** clock edge, modulo 4096. Active-high asynchronous reset clears all outputs. [Manual][4040]. |
| `4094` | Strobe **1**, serial data **2**, clock **3**, output enable **15**. | Parallel bits 0..7: **4, 5, 6, 7, 14, 13, 12, 11**. Serial QS **9**, delayed QS-prime **10**. Shifts on the positive edge. The parallel latch follows the shift register while strobe is high and holds while low. Enable is **active high**; disabling it floats only the parallel outputs. QS-prime samples the last shift stage on the negative edge. [Manual][4094]. |
| `4512` | Data channels 0..7: **1, 2, 3, 4, 5, 6, 7, 9**. Address A, B, C: **11, 12, 13**. Inhibit **10**, output enable **15**. | Selected data **14**. Output enable is **active low**; a high level floats the output. With output enabled, high inhibit forces zero. An unknown address still resolves a bit when all possible selected channels agree. [Manual][4512]. |
| `4514` | Strobe **1**, address bits 0..3: **2, 3, 21, 22**, inhibit **23**. | Decoded lines 0..15: **11, 9, 10, 8, 7, 6, 5, 4, 18, 17, 20, 19, 14, 13, 16, 15**. One active-high selected line. Address latch is transparent while strobe is high and holds while low. High inhibit forces every output low. Unknown address bits leave impossible lines definitively low. [Manual][4514]. |
| `4801` | Address bits 0..9: **8, 7, 6, 5, 4, 3, 2, 1, 23, 22**. Chip enable **18**, output enable **20**, write enable **21**. | Bidirectional data bits 0..7: **9, 10, 11, 13, 14, 15, 16, 17**. All three controls are **active low**. Capacity: 1024 bytes. [Manual][4801]. |
| `2716` | Address bits 0..10: **8, 7, 6, 5, 4, 3, 2, 1, 23, 22, 19**. Chip enable **18**, output enable **20**. | Data bits 0..7: **9, 10, 11, 13, 14, 15, 16, 17**. Both enables are **active low**. Capacity: 2048 bytes. Pin **19 is A10**, despite an incorrect ignored-pin label in the supplied example's ASCII drawing. Pin 21 is the programming supply and is ignored digitally. [Manual][2716]. |
| `logger` | Data bits 0..7: **1..8**, clock **9**, inhibit **10**. | Appends one byte to `./log.bin` on a definite positive edge when inhibit is false and all eight data bits are known. No output pin. Contract: comments in the supplied `logger.nts` fixture and the project subject. |

## Memory and files

| Element | Policy |
|---|---|
| RAM | Sample writes, commit when CE/WE are low; OE does not prevent writing |
| RAM bus | Floats while writing, deselected, or OE high; external drivers must release the bus before reads |
| U address/control | Bitwise merge of possibilities; agreeing bits remain known |
| ROM | `./rom.bin` at startup: absent → U; short → unknown tail; nonregular/unreadable/>2,048 bytes → error |
| Logger | Binary append to `./log.bin` on known rising edge, inhibit=0, eight known bits; I/O errors reported |

Tests cover 512 additions, 4,096 counter states, 1,024 RAM addresses, 2,048 ROM addresses, edges/cascades/U/high impedance/file errors. [Test guide](testing.md).

## Manufacturer manuals

[4001][4001] · [4011][4011] · [4030][4030] · [4069][4069] · [4071][4071] · [4081][4081]. Advanced-device manuals are linked in the table.

[4008]: datasheets/4008_adder.pdf
[4013]: datasheets/4013_flipflop.pdf
[4017]: datasheets/4017_johnson.pdf
[4040]: datasheets/4040_counter.pdf
[4094]: datasheets/4094_shift.pdf
[4512]: datasheets/4512_selector.pdf
[4514]: datasheets/4514_decoder.pdf
[4801]: datasheets/4801_ram.pdf
[2716]: datasheets/2716_rom.pdf
[4001]: datasheets/4001_nor.pdf
[4011]: datasheets/4011_nand.pdf
[4030]: datasheets/4030_xor.pdf
[4069]: datasheets/4069_not.pdf
[4071]: datasheets/4071_or.pdf
[4081]: datasheets/4081_and.pdf
