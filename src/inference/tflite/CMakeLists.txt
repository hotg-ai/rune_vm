cmake_minimum_required(VERSION 3.15)

# Options
message(STATUS "RUNE_VM_TFLITE_EXTERNAL=${RUNE_VM_TFLITE_EXTERNAL}")
if(RUNE_VM_TFLITE_EXTERNAL)
    set(RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS "" CACHE STRING "Tensorflow-lite-c include dirs")
    set(RUNE_VM_INFERENCE_TFLITE_LIBRARIES "" CACHE STRING "Tensorflow-lite-c libraries")

    message(STATUS "Tflite include dirs: ${RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS}")
    message(STATUS "Tflite libraries: ${RUNE_VM_INFERENCE_TFLITE_LIBRARIES}")
else()
    set(RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS "")
    set(RUNE_VM_INFERENCE_TFLITE_LIBRARIES tensorflowlite_c)
endif()

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TfLiteRuntime.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/TfLiteRuntime.hpp
)

# Add target
add_library(${RUNE_VM_INFERENCE_IMPL_TARGET} STATIC ${SOURCES} ${HEADERS})
set(RUNE_VM_STATIC_LIBRARIES ${RUNE_VM_STATIC_LIBRARIES} ${RUNE_VM_INFERENCE_IMPL_TARGET} CACHE INTERNAL "")

# Link public dependencies
target_link_libraries(
    ${RUNE_VM_INFERENCE_IMPL_TARGET}
    PUBLIC
    fmt::fmt)

# Link private dependencies
target_link_libraries(
    ${RUNE_VM_INFERENCE_IMPL_TARGET}
    PRIVATE
    ${RUNE_VM_INFERENCE_TFLITE_LIBRARIES})

# Add public include directories
target_include_directories(
    ${RUNE_VM_INFERENCE_IMPL_TARGET}
    PUBLIC
    ${RUNE_VM_INCLUDES_DIR}
    ${RUNE_VM_SRC_DIR})

# Add private include directories
target_include_directories(
    ${RUNE_VM_INFERENCE_IMPL_TARGET}
    PRIVATE
    ${RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS})
