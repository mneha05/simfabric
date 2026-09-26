#include <systemc>
#include <tlm>
#include <tlm_utils/simple_initiator_socket.h>
#include <tlm_utils/simple_target_socket.h>

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "simfabric/model.hpp"

using namespace sc_core;
using namespace tlm;

namespace {

struct WorkloadPacket {
  std::uint64_t flops;
  std::uint64_t bytes;
  double l2_hit_rate;
  int tiles;
  char name[32];
};

struct HbmMemory : sc_module {
  tlm_utils::simple_target_socket<HbmMemory> target{"target"};
  simfabric::Config cfg;

  SC_CTOR(HbmMemory) {
    target.register_b_transport(this, &HbmMemory::b_transport);
  }

  void b_transport(tlm_generic_payload& tx, sc_time& delay) {
    auto* p = reinterpret_cast<WorkloadPacket*>(tx.get_data_ptr());
    const double miss = static_cast<double>(p->bytes) * (1.0 - p->l2_hit_rate);
    const double gbps = cfg.dram_gbps * cfg.dram_efficiency;
    const double seconds = miss == 0.0 ? 0.0 : miss / (gbps * 1e9);
    delay += sc_time(seconds, SC_SEC);
    tx.set_response_status(TLM_OK_RESPONSE);
  }
};

struct Accelerator : sc_module {
  tlm_utils::simple_target_socket<Accelerator> target{"target"};
  tlm_utils::simple_initiator_socket<Accelerator> memory{"memory"};
  simfabric::Config cfg;

  SC_CTOR(Accelerator) {
    target.register_b_transport(this, &Accelerator::b_transport);
  }

  void b_transport(tlm_generic_payload& tx, sc_time& delay) {
    auto* p = reinterpret_cast<WorkloadPacket*>(tx.get_data_ptr());

    simfabric::Workload workload{
        p->name,
        p->flops,
        p->bytes,
    };
    auto local_cfg = cfg;
    local_cfg.l2_hit_rate = p->l2_hit_rate;
    local_cfg.tiles = p->tiles;
    const auto analytic = simfabric::simulate(local_cfg, workload);

    tlm_generic_payload mem_tx;
    mem_tx.set_command(TLM_READ_COMMAND);
    mem_tx.set_address(0);
    mem_tx.set_data_ptr(reinterpret_cast<unsigned char*>(p));
    mem_tx.set_data_length(sizeof(*p));
    mem_tx.set_streaming_width(sizeof(*p));
    sc_time dram_delay = SC_ZERO_TIME;
    memory->b_transport(mem_tx, dram_delay);

    const double l2_seconds =
        (static_cast<double>(p->bytes) * p->l2_hit_rate) /
        (cfg.l2_gbps * 1e9);
    const double memory_seconds = l2_seconds + dram_delay.to_seconds();
    const double compute_seconds =
        static_cast<double>(p->flops) / (cfg.compute_tflops * 1e12);
    const double arbitration = 1.0 + 0.015 * std::max(0, p->tiles - 1);
    const double modeled_seconds =
        std::max(compute_seconds, memory_seconds) * arbitration;

    delay += sc_time(modeled_seconds, SC_SEC);

    // The independent analytical path and TLM path should agree exactly.
    const double delta_ns =
        std::abs(modeled_seconds - analytic.total_ms / 1e3) * 1e9;
    if (delta_ns > 0.5) {
      SC_REPORT_ERROR("SimFabric", "analytical/TLM timing diverged");
    }

    tx.set_response_status(TLM_OK_RESPONSE);
  }
};

struct Driver : sc_module {
  tlm_utils::simple_initiator_socket<Driver> initiator{"initiator"};
  simfabric::Config cfg;
  std::vector<simfabric::Workload> workloads;

  SC_CTOR(Driver) {
    SC_THREAD(run);
  }

  void run() {
    for (const auto& w : workloads) {
      WorkloadPacket packet{};
      packet.flops = w.flops;
      packet.bytes = w.bytes;
      packet.l2_hit_rate = cfg.l2_hit_rate;
      packet.tiles = cfg.tiles;
      std::strncpy(packet.name, w.name.c_str(), sizeof(packet.name) - 1);

      tlm_generic_payload tx;
      tx.set_command(TLM_WRITE_COMMAND);
      tx.set_address(0x1000);
      tx.set_data_ptr(reinterpret_cast<unsigned char*>(&packet));
      tx.set_data_length(sizeof(packet));
      tx.set_streaming_width(sizeof(packet));

      sc_time delay = SC_ZERO_TIME;
      initiator->b_transport(tx, delay);
      if (tx.is_response_error()) {
        SC_REPORT_FATAL("SimFabric", tx.get_response_string().c_str());
      }

      const auto predicted = simfabric::simulate(cfg, w);
      std::cout << std::fixed << std::setprecision(4)
                << "TLM workload=" << w.name
                << " annotated_ms=" << delay.to_seconds() * 1e3
                << " analytic_ms=" << predicted.total_ms
                << " cycles=" << predicted.cycles
                << " bottleneck=" << predicted.bottleneck << "\n";
      wait(delay);
    }
    sc_stop();
  }
};

}  // namespace

int sc_main(int, char**) {
  simfabric::Config cfg;
  cfg.clock_ghz = 1.5;
  cfg.dram_gbps = 273.0;
  cfg.dram_efficiency = 231.3 / 273.0;
  cfg.l2_gbps = 1600.0;
  cfg.l2_hit_rate = 0.15;
  cfg.compute_tflops = 80.0;
  cfg.tiles = 8;

  Driver driver{"driver"};
  Accelerator accel{"accelerator"};
  HbmMemory hbm{"hbm"};

  driver.cfg = cfg;
  accel.cfg = cfg;
  hbm.cfg = cfg;
  driver.workloads = {
      {"decode-4MiB", 2ull * 128 * 4096 * 128, 4ull * 1024 * 1024},
      {"decode-16MiB", 2ull * 128 * 16384 * 128, 16ull * 1024 * 1024},
      {"decode-64MiB", 2ull * 128 * 65536 * 128, 64ull * 1024 * 1024},
  };

  driver.initiator.bind(accel.target);
  accel.memory.bind(hbm.target);

  sc_start();
  std::cout << "SystemC/TLM simulation complete at " << sc_time_stamp() << "\n";
  return sc_report_handler::get_count(SC_ERROR) == 0 ? 0 : 1;
}
