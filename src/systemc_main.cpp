#include <systemc>
#include <tlm>
#include "simfabric/model.hpp"
using namespace sc_core;
SC_MODULE(AcceleratorModel) {
  sc_in<bool> clk;
  simfabric::Config cfg;
  simfabric::Workload wl;
  SC_CTOR(AcceleratorModel) { SC_THREAD(run); sensitive << clk.pos(); }
  void run() {
    wait();
    auto r=simfabric::simulate(cfg,wl);
    auto cycles=static_cast<unsigned long long>(r.total_ms*1e6*cfg.clock_ghz);
    std::cout << "[SystemC] " << wl.name << " cycles=" << cycles << " total_ms=" << r.total_ms << " bottleneck=" << r.bottleneck << "
";
    sc_stop();
  }
};
int sc_main(int, char**) { sc_clock clk("clk", sc_time(1,SC_NS)); AcceleratorModel dut("accel"); dut.clk(clk); sc_start(); return 0; }
