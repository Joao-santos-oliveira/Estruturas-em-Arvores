CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O3 -Iinclude
SRC_DIR = src
INC_DIR = include
TEST_DIR = tests
VIS_DIR = visualizer
BENCH_DIR = benchmarks
BIN_DIR = bin

TARGETS = $(BIN_DIR)/main $(BIN_DIR)/test_trees $(BIN_DIR)/generate_traces $(BIN_DIR)/benchmark_runner

.PHONY: all clean test traces bench run

all: $(TARGETS)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

HEADERS = $(wildcard $(INC_DIR)/*.hpp)

$(BIN_DIR)/main: $(SRC_DIR)/main.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/test_trees: $(TEST_DIR)/test_trees.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/generate_traces: $(VIS_DIR)/generate_traces.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/benchmark_runner: $(BENCH_DIR)/benchmark_runner.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(BIN_DIR)/test_trees
	./$(BIN_DIR)/test_trees

traces: $(BIN_DIR)/generate_traces
	./$(BIN_DIR)/generate_traces

bench: $(BIN_DIR)/benchmark_runner
	./$(BIN_DIR)/benchmark_runner
	python3 benchmarks/plot_results.py

clean:
	rm -rf $(BIN_DIR) assets/*.dot assets/*.png relatorio/figuras/*.png
