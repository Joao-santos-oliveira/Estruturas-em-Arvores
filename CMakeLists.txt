cmake_minimum_required(VERSION 3.16)
project(EstruturasEmArvoresAEDS2 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

include_directories(include)

# Executável principal (Demonstração / Menu interativo)
add_executable(main src/main.cpp)

# Gerador de visualizações e rastreamento (Graphviz DOT)
add_executable(generate_traces visualizer/generate_traces.cpp)

# Suite de testes
add_executable(test_trees tests/test_trees.cpp)

# Benchmarking
add_executable(benchmark_runner benchmarks/benchmark_runner.cpp)
