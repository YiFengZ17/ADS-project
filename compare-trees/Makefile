CC = gcc
PYTHON ?= python3
CFLAGS = -std=c11 -O2 -Wall -Wextra -Wpedantic
CODE = code
SOURCES = $(CODE)/test.c
INCLUDED = $(CODE)/bst.c $(CODE)/avl.c $(CODE)/splay.c $(CODE)/redblack.c $(CODE)/bplus.c
BIN = build/compare-trees
SANITIZE_BIN = build/compare-trees-sanitize

all: $(BIN)

$(BIN): $(SOURCES) $(INCLUDED)
	mkdir -p build
	$(CC) $(CFLAGS) $(SOURCES) -o $@

check: $(BIN)
	./$(BIN) --check

sanitize: $(SOURCES) $(INCLUDED)
	mkdir -p build
	$(CC) -std=c11 -O1 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer $(SOURCES) -o $(SANITIZE_BIN)
	./$(SANITIZE_BIN) --check

benchmark: $(BIN)
	mkdir -p data/results data/inputs
	./$(BIN) --bench data/results/raw.csv data/inputs

analyze:
	$(PYTHON) scripts/analyze.py

plots: analyze
	$(PYTHON) scripts/plot_results.py

analysis-check:
	$(PYTHON) scripts/test_analysis.py

report draft:
	$(PYTHON) scripts/build_report.py --compile

.PHONY: all check sanitize benchmark analyze report draft package-code plots analysis-check


# Only sync source code after verification; PDFs require separate review.
package-code: check
	$(PYTHON) scripts/package_code.py
