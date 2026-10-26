cmake_minimum_required(VERSION 3.16)
project(CaptainMajid LANGUAGES CXX)

add_library(captain_core engine/src/action_strength.cpp engine/src/session.cpp)
target_include_directories(captain_core PUBLIC engine/include)
target_compile_features(captain_core PUBLIC cxx_std_17)
set_target_properties(captain_core PROPERTIES CXX_EXTENSIONS OFF)

add_executable(captain-prototype engine/src/main.cpp)
target_link_libraries(captain-prototype PRIVATE captain_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(core-tests engine/tests/core_tests.cpp)
    target_link_libraries(core-tests PRIVATE captain_core)
    add_test(NAME core COMMAND core-tests)
    find_package(Python3 COMPONENTS Interpreter QUIET)
    if(Python3_Interpreter_FOUND)
        add_test(NAME cli COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/check_cli.py
            --binary $<TARGET_FILE:captain-prototype>)
        add_test(NAME replica_loopback COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/check_replica_sync.py
            --binary $<TARGET_FILE:captain-prototype>)
        add_test(NAME adapter_failures COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/engine/tests/check_adapter.py
            --binary $<TARGET_FILE:captain-prototype>)
        set_tests_properties(cli replica_loopback adapter_failures PROPERTIES TIMEOUT 45)
    endif()
endif()
