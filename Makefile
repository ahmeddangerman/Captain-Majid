CXX ?= c++
CPPFLAGS += -Iengine/include
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CORE = engine/src/action_strength.cpp engine/src/session.cpp engine/src/defender_contact.cpp engine/src/shot_power.cpp engine/src/goalkeeper.cpp
CORE += engine/src/energy.cpp engine/src/keeper_state.cpp engine/src/reference_random.cpp
CORE += engine/src/shot_outcome.cpp engine/src/shot_sequence.cpp engine/src/defender_sequence.cpp engine/src/defender_outcome.cpp
CORE += engine/src/ball_recovery.cpp engine/src/field_handoff.cpp
CORE += engine/src/defender_selection.cpp
CORE += engine/src/field_defender_preparation.cpp engine/src/field_player_profile.cpp engine/src/prepared_defender_sequence.cpp engine/src/prepared_field_shot.cpp engine/src/prepared_attack.cpp engine/src/prepared_keeper.cpp engine/src/complete_prepared_shot.cpp
RESEARCH = engine/src/research_protocol.cpp
HEADERS = $(wildcard engine/include/captain/*.hpp)

.PHONY: all check
all: build/captain-prototype

build:
	mkdir -p build

build/captain-prototype: engine/src/main.cpp $(CORE) $(RESEARCH) engine/src/research_protocol.hpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/src/main.cpp $(RESEARCH) $(CORE) -o $@

build/core-tests: engine/tests/core_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/core_tests.cpp $(CORE) -o $@

build/defender-tests: engine/tests/defender_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/defender_tests.cpp $(CORE) -o $@

build/shot-tests: engine/tests/shot_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/shot_tests.cpp $(CORE) -o $@

build/energy-tests: engine/tests/energy_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/energy_tests.cpp $(CORE) -o $@

build/keeper-state-tests: engine/tests/keeper_state_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/keeper_state_tests.cpp $(CORE) -o $@

build/random-tests: engine/tests/random_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/random_tests.cpp $(CORE) -o $@

build/shot-sequence-tests: engine/tests/shot_sequence_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/shot_sequence_tests.cpp $(CORE) -o $@

build/defender-sequence-tests: engine/tests/defender_sequence_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/defender_sequence_tests.cpp $(CORE) -o $@

build/defender-outcome-tests: engine/tests/defender_outcome_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/defender_outcome_tests.cpp $(CORE) -o $@

build/ball-recovery-tests: engine/tests/ball_recovery_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/ball_recovery_tests.cpp $(CORE) -o $@

build/field-handoff-tests: engine/tests/field_handoff_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/field_handoff_tests.cpp $(CORE) -o $@

build/defender-selection-tests: engine/tests/defender_selection_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/defender_selection_tests.cpp $(CORE) -o $@

build/field-defender-preparation-tests: engine/tests/field_defender_preparation_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/field_defender_preparation_tests.cpp $(CORE) -o $@

build/field-player-profile-tests: engine/tests/field_player_profile_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/field_player_profile_tests.cpp $(CORE) -o $@

build/prepared-defender-sequence-tests: engine/tests/prepared_defender_sequence_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/prepared_defender_sequence_tests.cpp $(CORE) -o $@

build/prepared-field-shot-tests: engine/tests/prepared_field_shot_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/prepared_field_shot_tests.cpp $(CORE) -o $@

build/prepared-attack-tests: engine/tests/prepared_attack_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/prepared_attack_tests.cpp $(CORE) -o $@

build/prepared-keeper-tests: engine/tests/prepared_keeper_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/prepared_keeper_tests.cpp $(CORE) -o $@

build/complete-prepared-shot-tests: engine/tests/complete_prepared_shot_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/complete_prepared_shot_tests.cpp $(CORE) -o $@

check: build/complete-prepared-shot-tests build/prepared-keeper-tests build/prepared-attack-tests build/prepared-field-shot-tests build/prepared-defender-sequence-tests build/field-player-profile-tests build/core-tests build/defender-tests build/shot-tests build/energy-tests build/keeper-state-tests build/random-tests build/shot-sequence-tests build/defender-sequence-tests build/defender-outcome-tests build/ball-recovery-tests build/field-handoff-tests build/defender-selection-tests build/field-defender-preparation-tests build/captain-prototype
	./build/complete-prepared-shot-tests
	./build/prepared-keeper-tests
	./build/prepared-attack-tests
	./build/prepared-field-shot-tests
	./build/prepared-defender-sequence-tests
	./build/field-player-profile-tests
	./build/core-tests
	./build/defender-tests
	./build/shot-tests
	./build/energy-tests
	./build/keeper-state-tests
	./build/random-tests
	./build/shot-sequence-tests
	./build/defender-sequence-tests
	./build/defender-outcome-tests
	./build/ball-recovery-tests
	./build/field-handoff-tests
	./build/defender-selection-tests
	./build/field-defender-preparation-tests
	python3 engine/tests/check_cli.py
	python3 engine/tests/check_shot_cli.py
	python3 engine/tests/check_state_cli.py
	python3 engine/tests/check_preparation_cli.py
	python3 engine/tests/check_adapter.py
	python3 tools/check_replica_sync.py
