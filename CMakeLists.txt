cmake_minimum_required(VERSION 3.16)
project(CaptainMajid LANGUAGES CXX)

add_library(captain_core engine/src/action_strength.cpp engine/src/session.cpp
    engine/src/defender_contact.cpp engine/src/shot_power.cpp engine/src/goalkeeper.cpp
    engine/src/energy.cpp engine/src/keeper_state.cpp engine/src/reference_random.cpp
    engine/src/shot_outcome.cpp engine/src/shot_sequence.cpp engine/src/defender_sequence.cpp
    engine/src/defender_outcome.cpp engine/src/ball_recovery.cpp engine/src/field_handoff.cpp
    engine/src/defender_selection.cpp engine/src/field_defender_preparation.cpp
    engine/src/field_player_profile.cpp engine/src/prepared_defender_sequence.cpp)
target_include_directories(captain_core PUBLIC engine/include)
target_compile_features(captain_core PUBLIC cxx_std_17)
set_target_properties(captain_core PROPERTIES CXX_EXTENSIONS OFF)

add_executable(captain-prototype engine/src/main.cpp engine/src/research_protocol.cpp)
target_link_libraries(captain-prototype PRIVATE captain_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(core-tests engine/tests/core_tests.cpp)
    target_link_libraries(core-tests PRIVATE captain_core)
    add_test(NAME core COMMAND core-tests)
    add_executable(defender-tests engine/tests/defender_tests.cpp)
    target_link_libraries(defender-tests PRIVATE captain_core)
    add_test(NAME defender COMMAND defender-tests)
    add_executable(shot-tests engine/tests/shot_tests.cpp)
    target_link_libraries(shot-tests PRIVATE captain_core)
    add_test(NAME shot COMMAND shot-tests)
    foreach(case IN ITEMS energy keeper_state random)
        add_executable(${case}-tests engine/tests/${case}_tests.cpp)
        target_link_libraries(${case}-tests PRIVATE captain_core)
        add_test(NAME ${case} COMMAND ${case}-tests)
    endforeach()
    add_executable(shot-sequence-tests engine/tests/shot_sequence_tests.cpp)
    target_link_libraries(shot-sequence-tests PRIVATE captain_core)
    add_test(NAME shot_sequence COMMAND shot-sequence-tests)
    add_executable(defender-sequence-tests engine/tests/defender_sequence_tests.cpp)
    target_link_libraries(defender-sequence-tests PRIVATE captain_core)
    add_test(NAME defender_sequence COMMAND defender-sequence-tests)
    add_executable(defender-outcome-tests engine/tests/defender_outcome_tests.cpp)
    target_link_libraries(defender-outcome-tests PRIVATE captain_core)
    add_test(NAME defender_outcome COMMAND defender-outcome-tests)
    add_executable(ball-recovery-tests engine/tests/ball_recovery_tests.cpp)
    target_link_libraries(ball-recovery-tests PRIVATE captain_core)
    add_test(NAME ball_recovery COMMAND ball-recovery-tests)
    add_executable(field-handoff-tests engine/tests/field_handoff_tests.cpp)
    target_link_libraries(field-handoff-tests PRIVATE captain_core)
    add_test(NAME field_handoff COMMAND field-handoff-tests)
    add_executable(defender-selection-tests engine/tests/defender_selection_tests.cpp)
    target_link_libraries(defender-selection-tests PRIVATE captain_core)
    add_test(NAME defender_selection COMMAND defender-selection-tests)
    add_executable(field-defender-preparation-tests engine/tests/field_defender_preparation_tests.cpp)
    target_link_libraries(field-defender-preparation-tests PRIVATE captain_core)
    add_test(NAME field_defender_preparation COMMAND field-defender-preparation-tests)
    add_executable(field-player-profile-tests engine/tests/field_player_profile_tests.cpp)
    target_link_libraries(field-player-profile-tests PRIVATE captain_core)
    add_test(NAME field_player_profile COMMAND field-player-profile-tests)
    add_executable(prepared-defender-sequence-tests engine/tests/prepared_defender_sequence_tests.cpp)
    target_link_libraries(prepared-defender-sequence-tests PRIVATE captain_core)
    add_test(NAME prepared_defender_sequence COMMAND prepared-defender-sequence-tests)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME cli COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/check_cli.py
            --binary $<TARGET_FILE:captain-prototype>)
        add_test(NAME shot_cli COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/check_shot_cli.py
            --binary $<TARGET_FILE:captain-prototype>)
        add_test(NAME state_cli COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/check_state_cli.py
            --binary $<TARGET_FILE:captain-prototype>)
        add_test(NAME preparation_cli COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/check_preparation_cli.py
            --binary $<TARGET_FILE:captain-prototype>)
        add_test(NAME replica_loopback COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_replica_sync.py
            --binary $<TARGET_FILE:captain-prototype>)
        add_test(NAME adapter_failures COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/check_adapter.py
            --binary $<TARGET_FILE:captain-prototype>)
        set_tests_properties(cli shot_cli state_cli preparation_cli replica_loopback adapter_failures PROPERTIES TIMEOUT 45)
    endif()
endif()
