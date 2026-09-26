#include "simfabric/model.hpp"

#include <algorithm>
#include <cmath>

namespace simfabric {

Result simulate(const Config& c, const Workload& w) {
  const double hit = std::clamp(c.l2_hit_rate, 0.0, 1.0);
  const double efficiency = std::clamp(c.dram_efficiency, 0.01, 1.0);

  const double compute_s =
      static_cast<double>(w.flops) / (c.compute_tflops * 1e12);

  const double l2_bytes = static_cast<double>(w.bytes) * hit;
  const double dram_bytes = static_cast<double>(w.bytes) * (1.0 - hit);

  const double l2_s = l2_bytes / (c.l2_gbps * 1e9);
  const double dram_effective_gbps = c.dram_gbps * efficiency;
  const double dram_s =
      dram_bytes == 0.0 ? 0.0 : dram_bytes / (dram_effective_gbps * 1e9);
  const double memory_s = l2_s + dram_s;

  // A compact contention model for shared arbitration as tile count grows.
  const double arbitration = 1.0 + 0.015 * std::max(0, c.tiles - 1);
  const double total_s = std::max(compute_s, memory_s) * arbitration;

  Result r;
  r.compute_ms = compute_s * 1e3;
  r.l2_ms = l2_s * 1e3;
  r.dram_ms = dram_s * 1e3;
  r.memory_ms = memory_s * 1e3;
  r.total_ms = total_s * 1e3;
  r.achieved_tflops =
      total_s > 0.0 ? (static_cast<double>(w.flops) / total_s) / 1e12 : 0.0;
  r.effective_dram_gbps = dram_effective_gbps;
  r.dram_util = efficiency;
  r.cycles = static_cast<std::uint64_t>(
      std::llround(total_s * c.clock_ghz * 1e9));
  r.bottleneck = compute_s >= memory_s ? "compute" : "memory";
  return r;
}

std::vector<Result> sweep(const Workload& w, const std::vector<Config>& cs) {
  std::vector<Result> out;
  out.reserve(cs.size());
  for (const auto& c : cs) out.push_back(simulate(c, w));
  return out;
}

}  // namespace simfabric
