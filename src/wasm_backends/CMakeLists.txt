cmake_minimum_required(VERSION 3.15)

# Options
# TODO: considering runtime interface it should be a list
set(RUNE_VM_WASM_BACKEND "wasm3" CACHE STRING "WebAssembly backend to build with")
set_property(CACHE RUNE_VM_WASM_BACKEND PROPERTY STRINGS wasm3)

# Sources & Headers
set(
    SOURCES
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Backends.hpp
)

add_library(${RUNE_VM_WASM_BACKEND_TARGET} INTERFACE)
#add_library(${RUNE_VM_WASM_BACKEND_TARGET} STATIC ${SOURCES} ${HEADERS})

# Include actual backend
set(RUNE_VM_WASM_BACKEND_IMPL_TARGET rune_vm_wasm_backend_impl)
if(${RUNE_VM_WASM_BACKEND} STREQUAL wasm3)
    message(STATUS "Wasm3 backend was chosen")
    add_subdirectory(wasm3)
else()
    message(ERROR "Unknown backend")
endif()

# Link actual backend
target_link_libraries(
    ${RUNE_VM_WASM_BACKEND_TARGET}
    INTERFACE
    fmt::fmt
    ${RUNE_VM_WASM_BACKEND_IMPL_TARGET})
