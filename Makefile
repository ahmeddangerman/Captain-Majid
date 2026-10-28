CXX ?= c++
CPPFLAGS += -Iengine/include
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic
CORE = engine/src/action_strength.cpp engine/src/session.cpp engine/src/defender_contact.cpp engine/src/shot_power.cpp engine/src/goalkeeper.cpp
CORE += engine/src/energy.cpp engine/src/keeper_state.cpp engine/src/reference_random.cpp
CORE += engine/src/shot_outcome.cpp engine/src/shot_sequence.cpp engine/src/defender_sequence.cpp engine/src/defender_outcome.cpp
CORE += engine/src/ball_recovery.cpp engine/src/field_handoff.cpp
CORE += engine/src/defender_selection.cpp
CORE += engine/src/field_defender_preparation.cpp engine/src/field_player_profile.cpp engine/src/prepared_defender_sequence.cpp engine/src/prepared_field_shot.cpp engine/src/prepared_attack.cpp engine/src/prepared_keeper.cpp engine/src/complete_prepared_shot.cpp engine/src/field_comparison_content.cpp engine/src/field_interruption.cpp
RESEARCH = engine/src/research_protocol.cpp
CORE += engine/src/shot_content.cpp
CORE += engine/src/shot_scene.cpp
CORE += engine/src/shot_approach.cpp
CORE += engine/src/flight_contact.cpp
CORE += engine/src/player_movement.cpp
CORE += engine/src/movement_profile.cpp engine/src/field_targets.cpp engine/src/field_phase.cpp
CORE += engine/src/field_state_io.cpp
CORE += engine/src/terminal_flight_phase.cpp
CORE += engine/src/match_clock.cpp engine/src/contact_prelude.cpp
CORE += engine/src/phase_session.cpp engine/src/phase_session_io.cpp
CORE += engine/src/experience.cpp
CORE += engine/src/recovery_session.cpp
CORE += engine/src/control_session.cpp
CORE += engine/src/control_input.cpp
CORE += engine/src/control_tick.cpp
CORE += engine/src/action_menu.cpp engine/src/session_shot.cpp engine/src/staged_shot.cpp engine/src/selected_dispatch.cpp engine/src/capture_clearance.cpp engine/src/clearance_race.cpp engine/src/clearance_handoff.cpp engine/src/restart_route.cpp engine/src/restart_options.cpp
HEADERS = $(wildcard engine/include/captain/*.hpp)

.PHONY: all check
all: build/captain-prototype build/captain-shot-lab build/captain-field-lab build/captain-phase-session

build:
	mkdir -p build

build/captain-prototype: engine/src/main.cpp $(CORE) $(RESEARCH) engine/src/research_protocol.hpp $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/src/main.cpp $(RESEARCH) $(CORE) -o $@

build/captain-shot-lab: engine/src/shot_lab.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/src/shot_lab.cpp $(CORE) -o $@

build/captain-field-lab: engine/src/field_lab.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/src/field_lab.cpp $(CORE) -o $@

build/captain-phase-session: engine/src/phase_session_lab.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/src/phase_session_lab.cpp $(CORE) -o $@

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

build/field-comparison-content-tests: engine/tests/field_comparison_content_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/field_comparison_content_tests.cpp $(CORE) -o $@

build/field-interruption-tests: engine/tests/field_interruption_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/field_interruption_tests.cpp $(CORE) -o $@

build/session-shot-tests: engine/tests/session_shot_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/session_shot_tests.cpp $(CORE) -o $@

build/staged-shot-tests: engine/tests/staged_shot_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/staged_shot_tests.cpp $(CORE) -o $@

build/selected-dispatch-tests: engine/tests/selected_dispatch_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/selected_dispatch_tests.cpp $(CORE) -o $@

build/selected-recovery-tests: engine/tests/selected_recovery_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/selected_recovery_tests.cpp $(CORE) -o $@

build/capture-clearance-tests: engine/tests/capture_clearance_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/capture_clearance_tests.cpp $(CORE) -o $@

build/clearance-race-tests: engine/tests/clearance_race_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/clearance_race_tests.cpp $(CORE) -o $@

build/clearance-handoff-tests: engine/tests/clearance_handoff_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/clearance_handoff_tests.cpp $(CORE) -o $@

build/restart-route-tests: engine/tests/restart_route_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/restart_route_tests.cpp $(CORE) -o $@

build/restart-options-tests: engine/tests/restart_options_tests.cpp $(CORE) $(HEADERS) | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) engine/tests/restart_options_tests.cpp $(CORE) -o $@

check: build/restart-options-tests build/restart-route-tests build/clearance-handoff-tests build/clearance-race-tests build/capture-clearance-tests build/selected-recovery-tests build/selected-dispatch-tests build/staged-shot-tests build/session-shot-tests build/field-interruption-tests build/field-comparison-content-tests build/complete-prepared-shot-tests build/prepared-keeper-tests build/prepared-attack-tests build/prepared-field-shot-tests build/prepared-defender-sequence-tests build/field-player-profile-tests build/core-tests build/defender-tests build/shot-tests build/energy-tests build/keeper-state-tests build/random-tests build/shot-sequence-tests build/defender-sequence-tests build/defender-outcome-tests build/ball-recovery-tests build/field-handoff-tests build/defender-selection-tests build/field-defender-preparation-tests build/captain-prototype
	./build/restart-options-tests
	./build/restart-route-tests
	./build/clearance-handoff-tests
	./build/clearance-race-tests
	./build/capture-clearance-tests
	./build/selected-recovery-tests
	./build/selected-dispatch-tests
	./build/staged-shot-tests
	./build/session-shot-tests
	./build/field-interruption-tests
	./build/field-comparison-content-tests
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
	python3 tools/check_reference_cartridge.py
