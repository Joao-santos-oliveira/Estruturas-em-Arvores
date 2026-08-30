CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O3 -Iinclude
INC_DIR = include
TEST_DIR = tests
BENCH_DIR = benchmarks
BIN_DIR = bin

HEADERS = $(wildcard $(INC_DIR)/*.hpp)

.PHONY: all clean test datasets bench bench-all

all: $(BIN_DIR)/test_trees $(BIN_DIR)/executar_benchmarks

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_DIR)/test_trees: $(TEST_DIR)/test_trees.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIN_DIR)/executar_benchmarks: $(BENCH_DIR)/executar_benchmarks.cpp $(HEADERS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(BIN_DIR)/test_trees
	./$(BIN_DIR)/test_trees

datasets:
	python3 benchmarks/datasets/gerar_datasets.py

bench: $(BIN_DIR)/executar_benchmarks
	./$(BIN_DIR)/executar_benchmarks

bench-all: $(BIN_DIR)/executar_benchmarks
	./$(BIN_DIR)/executar_benchmarks --all

clean:
	rm -rf $(BIN_DIR) benchmarks/dados_comparativos.csv
