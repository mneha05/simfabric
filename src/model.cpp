#include "simfabric/model.hpp"
#include <algorithm>
namespace simfabric {
Result simulate(const Config& c, const Workload& w) {
  const double compute_s = static_cast<double>(w.flops) / (c.compute_tflops * 1e12);
  const double effective_bw = c.l2_hit_rate*c.l2_gbps + (1.0-c.l2_hit_rate)*c.dram_gbps;
  const double memory_s = static_cast<double>(w.bytes) / (effective_bw * 1e9);
  const double arbitration = 1.0 + 0.015 * std::max(0, c.tiles - 1);
  const double total_s = std::max(compute_s, memory_s) * arbitration;
  Result r;
  r.compute_ms=compute_s*1e3; r.memory_ms=memory_s*1e3; r.total_ms=total_s*1e3;
  r.achieved_tflops=(static_cast<double>(w.flops)/total_s)/1e12;
  r.dram_util=std::min(1.0, (static_cast<double>(w.bytes)/total_s)/(c.dram_gbps*1e9));
  r.bottleneck = compute_s >= memory_s ? "compute" : "memory";
  return r;
}
std::vector<Result> sweep(const Workload& w, const std::vector<Config>& cs) { std::vector<Result> o; for (auto& c:cs) o.push_back(simulate(c,w)); return o; }
}
