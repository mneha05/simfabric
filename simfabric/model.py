from __future__ import annotations
from dataclasses import dataclass, asdict
from math import inf
@dataclass(frozen=True)
class Config:
    clock_ghz: float=1.5; dram_gbps: float=900.; l2_gbps: float=2400.; compute_tflops: float=60.; l2_hit_rate: float=.65; tiles:int=16
@dataclass(frozen=True)
class Workload:
    name:str="gemm-4096"; flops:int=2*4096*4096*4096; bytes:int=3*4096*4096*2
@dataclass
class Result:
    compute_ms:float; memory_ms:float; total_ms:float; achieved_tflops:float; dram_util:float; bottleneck:str
    def dict(self): return asdict(self)
def simulate(c:Config,w:Workload)->Result:
    compute_s=w.flops/(c.compute_tflops*1e12)
    bw=c.l2_hit_rate*c.l2_gbps+(1-c.l2_hit_rate)*c.dram_gbps
    memory_s=w.bytes/(bw*1e9)
    arbitration=1+.015*max(0,c.tiles-1)
    total=max(compute_s,memory_s)*arbitration
    return Result(compute_s*1e3,memory_s*1e3,total*1e3,(w.flops/total)/1e12,min(1,(w.bytes/total)/(c.dram_gbps*1e9)),"compute" if compute_s>=memory_s else "memory")
def design_space(w:Workload):
    out=[]
    for tiles in (4,8,16,32):
      for hit in (.25,.5,.75,.9):
        c=Config(tiles=tiles,l2_hit_rate=hit); r=simulate(c,w); out.append((c,r))
    return sorted(out,key=lambda x:x[1].total_ms)
