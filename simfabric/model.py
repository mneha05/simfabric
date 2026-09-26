from __future__ import annotations
from dataclasses import dataclass, asdict

@dataclass(frozen=True)
class Config:
    clock_ghz: float = 1.5
    dram_gbps: float = 900.0
    l2_gbps: float = 2400.0
    compute_tflops: float = 60.0
    l2_hit_rate: float = 0.65
    dram_efficiency: float = 0.80
    tiles: int = 16

@dataclass(frozen=True)
class Workload:
    name: str = "gemm-4096"
    flops: int = 2 * 4096 * 4096 * 4096
    bytes: int = 3 * 4096 * 4096 * 2

@dataclass
class Result:
    compute_ms: float
    l2_ms: float
    dram_ms: float
    memory_ms: float
    total_ms: float
    achieved_tflops: float
    effective_dram_gbps: float
    dram_util: float
    cycles: int
    bottleneck: str

    def dict(self):
        return asdict(self)

def simulate(c: Config, w: Workload) -> Result:
    hit = min(1.0, max(0.0, c.l2_hit_rate))
    efficiency = min(1.0, max(0.01, c.dram_efficiency))

    compute_s = w.flops / (c.compute_tflops * 1e12)
    l2_bytes = w.bytes * hit
    dram_bytes = w.bytes * (1.0 - hit)
    l2_s = l2_bytes / (c.l2_gbps * 1e9)
    effective_dram_gbps = c.dram_gbps * efficiency
    dram_s = 0.0 if dram_bytes == 0 else dram_bytes / (effective_dram_gbps * 1e9)
    memory_s = l2_s + dram_s

    arbitration = 1.0 + 0.015 * max(0, c.tiles - 1)
    total_s = max(compute_s, memory_s) * arbitration

    return Result(
        compute_ms=compute_s * 1e3,
        l2_ms=l2_s * 1e3,
        dram_ms=dram_s * 1e3,
        memory_ms=memory_s * 1e3,
        total_ms=total_s * 1e3,
        achieved_tflops=(w.flops / total_s) / 1e12 if total_s else 0.0,
        effective_dram_gbps=effective_dram_gbps,
        dram_util=efficiency,
        cycles=round(total_s * c.clock_ghz * 1e9),
        bottleneck="compute" if compute_s >= memory_s else "memory",
    )

def design_space(w: Workload):
    out = []
    for tiles in (4, 8, 16, 32):
        for hit in (.25, .5, .75, .9):
            c = Config(tiles=tiles, l2_hit_rate=hit)
            r = simulate(c, w)
            out.append((c, r))
    return sorted(out, key=lambda x: x[1].total_ms)
