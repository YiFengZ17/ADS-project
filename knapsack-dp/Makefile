CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic
SANFLAGS = -std=c++17 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer
PYTHON ?= python3
.PHONY: all check sanitize benchmark report package-code
all: build/knapsack build/bench build/gen
build/knapsack: code/main.cpp code/knapsack.cpp code/knapsack.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) code/main.cpp code/knapsack.cpp -o $@
build/bench: code/bench.cpp code/knapsack.cpp code/knapsack.h
	mkdir -p build
	$(CXX) $(CXXFLAGS) code/bench.cpp code/knapsack.cpp -o $@
build/gen: code/gen.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@
check: all
	$(PYTHON) scripts/check.py --binary build/knapsack --gen build/gen
sanitize:
	mkdir -p build
	$(CXX) $(SANFLAGS) code/main.cpp code/knapsack.cpp -o build/knapsack-san
	$(PYTHON) scripts/check.py --binary build/knapsack-san --gen build/gen --quick
benchmark: all
	$(PYTHON) scripts/benchmark.py
report:
	$(PYTHON) scripts/build_report.py --compile
package-code: check
	$(PYTHON) scripts/package_code.py
