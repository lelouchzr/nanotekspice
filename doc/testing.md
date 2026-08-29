# Tests

| Suite | Coverage |
|---|---|
| Basic / advanced / parser | Ternary gates, CMOS pinouts, counters, RAM/ROM, shared nets, parsing and errors |
| CLI / signals / examples | Real shell protocol, Ctrl+C, exit codes and every published circuit |
| Bonus / Python oracle | Variable-precision arithmetic, SUBLEQ, keyboard and all 4,096 matrix pixels |
| Native GUI | SDL events, editing, stepping, running, zoom/pan and window lifecycle |
| Memory / sanitizers | Invalid memory accesses, lost blocks and undefined behavior |

```sh
make test
make BUILD_DIR=build-core BONUS=OFF test
cmake -S . -B build-asan -DNTS_SANITIZERS=ON -DNTS_BUILD_GRAPHICS=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-asan --parallel 4
ctest --test-dir build-asan --output-on-failure
```

Linux CI installs Valgrind and Xvfb. CMake registers `memory_*` tests automatically when Valgrind is available; the native GUI tests require SDL2 and Xvfb.

```sh
ctest --test-dir build -R '^memory_' --output-on-failure
```

Memcheck disables default suppressions. Core, CPU and CLI checks reject all remaining allocations. The GUI check reports third-party reachable caches while rejecting invalid accesses and all definite, indirect or possible lost blocks.

Builds use strict warnings as errors and compile headers independently. `source_quality` checks formatting when clang-format is installed. Generated logs and memory reports stay in ignored build directories; archived machine reports are excluded from the published source.
