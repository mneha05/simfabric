#include "simfabric/model.hpp"

#include <cassert>
#include <iostream>

int main() {
  simfabric::Config cfg;
  simfabric::Workload workload;
  const auto result = simfabric::simulate(cfg, workload);

  assert(result.total_ms > 0.0);
  assert(result.achieved_tflops > 0.0);
  assert(result.cycles > 0);
  assert(result.memory_ms >= result.l2_ms);
  assert(result.memory_ms >= result.dram_ms);

  std::cout << "PASS total_ms=" << result.total_ms
            << " tflops=" << result.achieved_tflops
            << " cycles=" << result.cycles << "\n";
  return 0;
}
