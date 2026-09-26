#!/usr/bin/env python3
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
data = json.loads((ROOT / "data/calibration.json").read_text())

print("SimFabric empirical bandwidth correlation")
print("GPU                         peak GB/s  measured GB/s  efficiency")
print("-" * 72)
for m in data["measurements"]:
    eff = m["measured_gbps"] / m["theoretical_gbps"]
    print(f'{m["gpu"]:<28} {m["theoretical_gbps"]:>9.1f} {m["measured_gbps"]:>13.1f} {eff:>10.3f}')

t4, gb10 = data["measurements"]
print("\nCalibration interpretation")
print(f'T4   : {t4["measured_gbps"]:.1f}/{t4["theoretical_gbps"]:.1f} = {t4["measured_gbps"]/t4["theoretical_gbps"]:.1%} of peak')
print(f'GB10 : {gb10["measured_gbps"]:.1f}/{gb10["theoretical_gbps"]:.1f} = {gb10["measured_gbps"]/gb10["theoretical_gbps"]:.1%} of peak')
print("\nThese are empirical anchors, not independent validation points.")
