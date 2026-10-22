CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Wpedantic
PYTHON ?= python3
.PHONY: all check sanitize benchmark report package-code
all: build/recover-design
build/recover-design: code/main.cpp
	mkdir -p build
	$(CXX) $(CXXFLAGS) $< -o $@
check: all
	$(PYTHON) scripts/check.py --binary build/recover-design
sanitize:
	mkdir -p build
	$(CXX) -std=c++17 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer -fno-pie -no-pie code/main.cpp -o build/recover-design-san
	$(PYTHON) scripts/check.py --binary build/recover-design-san
benchmark: all
	$(PYTHON) scripts/check.py --binary build/recover-design --benchmark
report:
	$(PYTHON) scripts/build_report.py --compile
package-code: check
	$(PYTHON) scripts/package_code.py
