cmake_minimum_required(VERSION 3.15)

# Sources & Headers
set(
    SOURCES
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/FormatCapabilitiesParameter.hpp
)

add_library(${RUNE_VM_IO_TARGET} INTERFACE)
#add_library(${RUNE_VM_IO_TARGET} STATIC ${SOURCES} ${HEADERS})

# Link private dependencies
target_link_libraries(
    ${RUNE_VM_IO_TARGET}
    INTERFACE
    fmt::fmt)

# Add include directories
target_include_directories(
    ${RUNE_VM_IO_TARGET}
    INTERFACE
    ${RUNE_VM_INCLUDES_DIR}
    ${RUNE_VM_SRC_DIR})