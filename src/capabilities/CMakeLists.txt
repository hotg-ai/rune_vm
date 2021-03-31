cmake_minimum_required(VERSION 3.15)

# Add internal subdirectories
set(RUNE_VM_CAPABILITIES_DELEGATES_TARGET rune_vm_capabilities_delegates)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/delegates)

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CapabilitiesDelegatesManager.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/CapabilitiesDelegatesManager.hpp
)

add_library(${RUNE_VM_CAPABILITIES_TARGET} STATIC ${SOURCES} ${HEADERS})

# Link private dependencies
target_link_libraries(
    ${RUNE_VM_CAPABILITIES_TARGET}
    PRIVATE
    ${RUNE_VM_CAPABILITIES_DELEGATES_TARGET})

# Add include directories
target_include_directories(
    ${RUNE_VM_CAPABILITIES_TARGET}
    PUBLIC
    ${RUNE_VM_INCLUDES_DIR}
    ${RUNE_VM_SRC_DIR})