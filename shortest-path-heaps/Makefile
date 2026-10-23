CXX ?= c++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic
all: build/benchmark
build/benchmark: code/main.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@
self-test: build/benchmark
	./build/benchmark --self-test
parser-test: build/benchmark
	python3 scripts/test_parser.py
attribute: build/benchmark
	python3 scripts/attribute.py data/USA-road-d.NY.gr 1000 NY
sanitizer:
	mkdir -p build
	$(CXX) -std=c++17 -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer code/main.cpp -o build/benchmark-san
	./build/benchmark-san --self-test
	BENCHMARK=build/benchmark-san python3 scripts/test_parser.py
.PHONY: all self-test parser-test attribute sanitizer
