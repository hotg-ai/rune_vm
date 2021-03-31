cmake_minimum_required(VERSION 3.15)

# Options
# TODO: considering runtime interface it should be a list
set(RUNE_VM_INFERENCE_BACKEND "tflite" CACHE STRING "Inference backend to build with")
set_property(CACHE RUNE_VM_INFERENCE_BACKEND PROPERTY STRINGS tflite)

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Inference.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/Inference.hpp
)

# Add target
add_library(${RUNE_VM_INFERENCE_BACKEND_TARGET} STATIC ${SOURCES} ${HEADERS})

# Include actual backend
set(RUNE_VM_INFERENCE_IMPL_TARGET rune_vm_inference_backend_impl)
if(${RUNE_VM_INFERENCE_BACKEND} STREQUAL tflite)
    message(STATUS "Tflite inference backend was chosen")
    add_subdirectory(tflite)
else()
    message(ERROR "Unknown backend")
endif()

# Link actual backend
target_link_libraries(
    ${RUNE_VM_INFERENCE_BACKEND_TARGET}
    PUBLIC
    fmt::fmt
    ${RUNE_VM_INFERENCE_IMPL_TARGET})
