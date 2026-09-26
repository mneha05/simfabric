import argparse,json
from pathlib import Path
from .model import Config,Workload,simulate,design_space
def main():
 p=argparse.ArgumentParser(prog="simfabric"); sp=p.add_subparsers(dest="cmd",required=True)
 s=sp.add_parser("run"); s.add_argument("--tiles",type=int,default=16); s.add_argument("--hit-rate",type=float,default=.65); s.add_argument("--json",action="store_true")
 sp.add_parser("sweep"); a=p.parse_args(); w=Workload()
 if a.cmd=="run":
  c=Config(tiles=a.tiles,l2_hit_rate=a.hit_rate); r=simulate(c,w)
  if a.json: print(json.dumps(r.dict(),indent=2))
  else:
   print(f"SimFabric · {w.name}"); print(f"tiles={c.tiles} l2_hit={c.l2_hit_rate:.2f}"); print(f"compute={r.compute_ms:.3f} ms memory={r.memory_ms:.3f} ms total={r.total_ms:.3f} ms"); print(f"achieved={r.achieved_tflops:.2f} TFLOP/s bottleneck={r.bottleneck}")
 else:
  print("rank  tiles  hit-rate  latency-ms  TFLOP/s")
  for i,(c,r) in enumerate(design_space(w)[:10],1): print(f"{i:>4} {c.tiles:>6} {c.l2_hit_rate:>9.2f} {r.total_ms:>11.3f} {r.achieved_tflops:>8.2f}")
