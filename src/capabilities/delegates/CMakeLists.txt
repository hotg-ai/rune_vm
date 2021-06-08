cmake_minimum_required(VERSION 3.15)

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/RandomCapabilityDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DelegateFactory.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/RandomCapabilityDelegate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DelegateFactory.hpp
)

add_library(${RUNE_VM_CAPABILITIES_DELEGATES_TARGET} STATIC ${SOURCES} ${HEADERS})
set(RUNE_VM_STATIC_LIBRARIES ${RUNE_VM_STATIC_LIBRARIES} ${RUNE_VM_CAPABILITIES_DELEGATES_TARGET} CACHE INTERNAL "")

# Link public dependencies
target_link_libraries(
    ${RUNE_VM_CAPABILITIES_DELEGATES_TARGET}
    PUBLIC
    ${RUNE_VM_IO_TARGET}
    ${RUNE_VM_TARGET_NAME})

# Add include directories
target_include_directories(
    ${RUNE_VM_CAPABILITIES_DELEGATES_TARGET}
    PUBLIC
    ${RUNE_VM_INCLUDES_DIR}
    ${RUNE_VM_SRC_DIR})
