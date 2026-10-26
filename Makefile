CXX ?= c++
CPPFLAGS += -Iengine/include
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CORE = engine/src/action_strength.cpp engine/src/session.cpp
HEADERS = $(wildcard engine/include/captain/*.hpp)

.PHONY: all check
all: build/captain-prototype

build:
	mkdir -p build

build/captain-prototype: engine/src/main.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/src/main.cpp $(CORE) -o $@

build/core-tests: engine/tests/core_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/core_tests.cpp $(CORE) -o $@

check: build/core-tests build/captain-prototype
	./build/core-tests
	python3 engine/tests/check_cli.py
	python3 engine/tests/check_adapter.py
	python3 tools/check_replica_sync.py
