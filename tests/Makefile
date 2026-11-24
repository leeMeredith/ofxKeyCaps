# Headless tests for src/core. No openFrameworks needed.
#   make test      build and run every suite (GREEN/RED per suite)
#   make headers   fail if src/core includes any openFrameworks header
#   make mutate    break each rule on purpose and check its suite goes RED
#   make clean

CORE ?= ../src/core
CXX ?= c++
CXXFLAGS ?= -std=c++17 -O1 -Wall -Wextra -Wpedantic -Werror
BUILD ?= build

SRCS := $(wildcard $(CORE)/*.cpp) test_main.cpp
BIN := $(BUILD)/keycaps_tests

.PHONY: test headers mutate clean

test: headers $(BIN)
	@"$(abspath $(BIN))"

$(BIN): $(SRCS) $(wildcard $(CORE)/*.h) data/probe_pass1_scroll.inc
	@mkdir -p $(BUILD)
	@$(CXX) $(CXXFLAGS) -I$(CORE) -I. $(SRCS) -o $@

headers:
	@if grep -nE '#include[[:space:]]*[<"](ofMain\.h|of[A-Z][A-Za-z0-9]*\.h)' $(CORE)/*.h $(CORE)/*.cpp; then \
		echo "HEADERS RED: src/core includes an openFrameworks header"; exit 1; \
	else echo "HEADERS GREEN: src/core has no openFrameworks includes"; fi

mutate:
	@./mutate.sh

clean:
	@rm -rf $(BUILD)
