#include "simfabric/model.hpp"
#include <cassert>
#include <iostream>
int main(){ simfabric::Config c; simfabric::Workload w; auto r=simfabric::simulate(c,w); assert(r.total_ms>0); assert(r.achieved_tflops>0); std::cout<<"PASS total_ms="<<r.total_ms<<" tflops="<<r.achieved_tflops<<"
"; }
