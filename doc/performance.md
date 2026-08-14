# Profiling

Shared nets maintain an index of output and bidirectional drivers. Input reads traverse drivers instead of repeatedly scanning every consumer. The index is updated when nets are created, merged or destroyed.

| Workload | Purpose |
|---|---|
| Fanout | Many gates share a small set of drivers |
| Counters | Sequential ticks and repeated output reads |
| Cascade | Deep combinational chains; regression control |

The benchmark checks deterministic output hashes. Tests cover conflicts, high impedance, merge/destruction, cycles and excluded bidirectional drivers.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DNTS_BENCHMARKS=ON
cmake --build build --target nts_benchmark --parallel 4
python3 bonus/benchmarks/reproduce.py --output /tmp/nts-reference
python3 bonus/benchmarks/compare.py --before /tmp/nts-reference --after build/bin/nts_benchmark --output /tmp/nts-comparison
```

The baseline resolver source is retained in `performance/baseline-source/` to reproduce comparisons without editing the checkout. Compare matching build options and alternating runs on an otherwise idle machine.

For native macOS stack sampling:

```sh
python3 bonus/benchmarks/profile.py --binary build/bin/nts_benchmark --output /tmp/nts-profile
```

Reports may contain local paths and machine details. Keep them outside the repository. Timings depend on the workload and environment; no machine-specific measurements are distributed.
