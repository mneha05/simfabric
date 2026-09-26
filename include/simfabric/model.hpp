#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace simfabric {
struct Config {
  double clock_ghz{1.5};
  double dram_gbps{900.0};
  double l2_gbps{2400.0};
  double compute_tflops{60.0};
  double l2_hit_rate{0.65};
  int tiles{16};
};
struct Workload {
  std::string name{"gemm"};
  std::uint64_t flops{2ull*4096*4096*4096};
  std::uint64_t bytes{3ull*4096*4096*2};
};
struct Result {
  double compute_ms{};
  double memory_ms{};
  double total_ms{};
  double achieved_tflops{};
  double dram_util{};
  std::string bottleneck;
};
Result simulate(const Config&, const Workload&);
std::vector<Result> sweep(const Workload&, const std::vector<Config>&);
}
