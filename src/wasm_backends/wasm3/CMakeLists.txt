cmake_minimum_required(VERSION 3.15)

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Rune.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Runtime.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Wasm3Rune.hpp
)

add_library(${RUNE_VM_WASM_BACKEND_IMPL_TARGET} STATIC ${SOURCES} ${HEADERS})
set(RUNE_VM_STATIC_LIBRARIES ${RUNE_VM_STATIC_LIBRARIES} ${RUNE_VM_WASM_BACKEND_IMPL_TARGET} CACHE INTERNAL "")

# Link public dependencies
target_link_libraries(
    ${RUNE_VM_WASM_BACKEND_IMPL_TARGET}
    PUBLIC
    fmt::fmt)

# Link private dependencies
target_link_libraries(
    ${RUNE_VM_WASM_BACKEND_IMPL_TARGET}
    PRIVATE
    m3
    mio)

# Add include directories
target_include_directories(
    ${RUNE_VM_WASM_BACKEND_IMPL_TARGET}
    PUBLIC
    ${RUNE_VM_INCLUDES_DIR}
    ${RUNE_VM_SRC_DIR})
