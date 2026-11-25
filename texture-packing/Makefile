CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic
SANFLAGS = -std=c++17 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer
PYTHON ?= python3
.PHONY: all check sanitize benchmark report package-code
all: build/pack build/gen
build/pack: code/main.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@
build/gen: code/gen.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@
check: all
	$(PYTHON) scripts/check.py --binary build/pack --gen build/gen
sanitize: build/gen
	mkdir -p build
	$(CXX) $(SANFLAGS) code/main.cpp -o build/pack-san
	$(PYTHON) scripts/check.py --binary build/pack-san --gen build/gen --quick
benchmark: all
	$(PYTHON) scripts/benchmark.py
report:
	$(PYTHON) scripts/build_report.py --compile
package-code: check
	$(PYTHON) scripts/package_code.py
