# SimFabric

<p align="center"><img src="docs/architecture.svg" width="96%" /></p>

<p align="center"><img src="docs/design-space.gif" width="92%"/></p>

**SystemC/TLM accelerator performance modeling and design-space exploration.**

SimFabric models a tiled accelerator as a compute roof, memory hierarchy, contention term and clocked SystemC wrapper. The same analytical core is available from Python for fast design-space sweeps and from C++ for embedding in a SystemC/TLM simulation.

```bash
pip install -e .
simfabric run --tiles 16 --hit-rate 0.65
simfabric sweep
```

## Why it exists

Performance modeling often starts before RTL exists. SimFabric lets an architect ask questions like: *Does doubling compute help if the workload is memory-bound? What L2 hit rate is needed before another tile pays off? Which configuration minimizes modeled latency?*

## SystemC path

```bash
cmake -S . -B build -G Ninja -DSIMFABRIC_WITH_SYSTEMC=ON
cmake --build build
./build/simfabric_systemc
```

The SystemC integration uses a clocked `SC_MODULE` and reuses the same analytical model, so the fast Python sweeps and event-driven simulation stay consistent.

## What is modeled

| Layer | Model |
|---|---|
| Compute | peak TFLOP/s roof |
| L2 | configurable hit rate + local bandwidth |
| DRAM/HBM | configurable GB/s |
| Tile scaling | arbitration/contention penalty |
| Timing | latency converted to clock cycles |
| Exploration | tile count × cache behavior sweep |

## Repo map

```text
include/simfabric/model.hpp   C++ model API
src/model.cpp                 analytical core
src/systemc_main.cpp          SystemC clocked wrapper
simfabric/                    Python explorer + CLI
tests/                        Python + C++ tests
docs/                         architecture + generated demo assets
```

## Validation

The default CI tests the pure C++ model and Python explorer on ordinary GitHub runners. The SystemC target is optional because runners do not ship SystemC by default. Current Accellera SystemC releases include TLM and the project targets the standard SystemC APIs rather than vendor-specific extensions.
