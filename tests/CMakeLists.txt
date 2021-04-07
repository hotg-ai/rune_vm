cmake_minimum_required(VERSION 3.15)

include(GoogleTest)

# Sources & Headers
set(
    TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/RuneTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuneVmEngineTests.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/RuneVmRuntimeTests.cpp
)
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TestLogger.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/TestLogger.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Common.hpp
)

# Add executable
set(RUNE_VM_TESTS_TARGET rune_vm_tests)
add_executable(${RUNE_VM_TESTS_TARGET} ${TESTS} ${SOURCES} ${HEADERS})

# Link dependencies
target_link_libraries(
    ${RUNE_VM_TESTS_TARGET}
    PRIVATE
    ${RUNE_VM_TARGET_NAME}
    gtest_main)

# Pass test data directory as define
target_compile_definitions(${RUNE_VM_TESTS_TARGET} PRIVATE TEST_DATA_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/test_data")

# Discover google tests to add to ctest
gtest_discover_tests(${RUNE_VM_TESTS_TARGET})
