cmake_minimum_required(VERSION 3.15)

include(GoogleTest)

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/RuneTests.cpp
)
set(
    HEADERS
)

# Add executable
set(RUNE_VM_TESTS_TARGET rune_vm_tests)
add_executable(${RUNE_VM_TESTS_TARGET} ${SOURCES} ${HEADERS})

# Link dependencies
target_link_libraries(
    ${RUNE_VM_TESTS_TARGET}
    PRIVATE
    gtest_main)

# Discover google tests to add to ctest
gtest_discover_tests(${RUNE_VM_TESTS_TARGET})
