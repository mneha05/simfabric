from simfabric.model import *
def test_more_compute_is_not_slower():
 w=Workload(); assert simulate(Config(compute_tflops=120),w).total_ms <= simulate(Config(compute_tflops=30),w).total_ms
def test_sweep_sorted():
 x=design_space(Workload()); assert x[0][1].total_ms <= x[-1][1].total_ms
