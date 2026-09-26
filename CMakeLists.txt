cmake_minimum_required(VERSION 3.20)
project(simfabric LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(simfabric_core src/model.cpp)
target_include_directories(simfabric_core PUBLIC include)

add_executable(simfabric_core_test tests/core_test.cpp)
target_link_libraries(simfabric_core_test PRIVATE simfabric_core)

option(SIMFABRIC_WITH_SYSTEMC "Build SystemC/TLM integration" OFF)
if(SIMFABRIC_WITH_SYSTEMC)
  find_package(SystemCLanguage CONFIG REQUIRED)
  add_executable(simfabric_systemc src/systemc_main.cpp)
  target_link_libraries(simfabric_systemc PRIVATE simfabric_core SystemC::systemc)
endif()
