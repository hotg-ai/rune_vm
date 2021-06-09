cmake_minimum_required(VERSION 3.15)
project(rune_vm)

# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Werror=return-type")
set(CMAKE_POSITION_INDEPENDENT_CODE True)

# Options
option(RUNE_VM_INSTALL "To install or not to install. Might be useful if you add this library as submodule dep" ON)
option(
    RUNE_VM_TFLITE_EXTERNAL
    "To link against external tflite binary. If set, you will have to provide RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS and RUNE_VM_INFERENCE_TFLITE_LIBRARIES too"
    OFF)
option(RUNE_VM_BUILD_TESTS "Build rune vm tests" ON)
option(RUNE_VM_BUILD_EXAMPLES "Build rune vm with examples" ON)

# Add external dependencies
## wasm3-c
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/extern/wasm3/source
    ${CMAKE_CURRENT_BINARY_DIR}/extern/wasm3_m3
    EXCLUDE_FROM_ALL)
## fmt
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/fmt ${CMAKE_CURRENT_BINARY_DIR}/extern/fmt EXCLUDE_FROM_ALL)
## mio
set(mio.installation OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/mio ${CMAKE_CURRENT_BINARY_DIR}/extern/mio EXCLUDE_FROM_ALL)
## json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/json ${CMAKE_CURRENT_BINARY_DIR}/extern/json EXCLUDE_FROM_ALL)

## tensorflow-lite-c
# TODO: considering runtime interface it should be a list
set(RUNE_VM_INFERENCE_BACKEND "tflite" CACHE STRING "Inference backend to build with")
set_property(CACHE RUNE_VM_INFERENCE_BACKEND PROPERTY STRINGS tflite)
message(STATUS "RUNE_VM_TFLITE_EXTERNAL=${RUNE_VM_TFLITE_EXTERNAL}")
if(RUNE_VM_TFLITE_EXTERNAL)
    set(RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS "" CACHE STRING "Tensorflow-lite-c include dirs")
    set(RUNE_VM_INFERENCE_TFLITE_LIBRARIES "" CACHE STRING "Tensorflow-lite-c libraries")

    message(STATUS "Tflite include dirs: ${RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS}")
    message(STATUS "Tflite libraries: ${RUNE_VM_INFERENCE_TFLITE_LIBRARIES}")
else()
    set(TFLITE_C_BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
    set(BUILD_TESTING OFF CACHE INTERNAL "")
    add_subdirectory(
        ${CMAKE_CURRENT_SOURCE_DIR}/extern/tensorflow/tensorflow/lite/c
        ${CMAKE_CURRENT_BINARY_DIR}/extern/tensorflow/tensorflow/lite/c
        EXCLUDE_FROM_ALL)
    set(RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/extern/tensorflow/)
    set(RUNE_VM_INFERENCE_TFLITE_LIBRARIES tensorflowlite_c)
endif(RUNE_VM_TFLITE_EXTERNAL)

# Add internal subdirectories
set(RUNE_VM_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(RUNE_VM_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities/delegates/RandomCapabilityDelegate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities/delegates/DelegateFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities/CapabilitiesDelegatesManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Rune.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Runtime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inference/tflite/TfLiteRuntime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inference/Inference.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inference/ModelManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RuneVm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Capabilities.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HostFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RuneInterop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Result.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OutputManager.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities/delegates/RandomCapabilityDelegate.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities/delegates/DelegateFactory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities/CapabilitiesDelegatesManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/Backends.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Engine.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Runtime.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends/wasm3/Wasm3Rune.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io/FormatCapabilitiesParameter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inference/tflite/TfLiteRuntime.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inference/ModelManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inference/Inference.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/RuneVm.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/Log.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/Capabilities.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/VirtualInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HostFunctions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RuneInterop.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Result.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/OutputManager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Common.hpp
)

set(RUNE_VM_WASM_BACKEND "wasm3" CACHE STRING "WebAssembly backend to build with")
set_property(CACHE RUNE_VM_WASM_BACKEND PROPERTY STRINGS wasm3)

# Platform specific variables
if(ANDROID)
    set(RUNE_VM_LIBRARY_DIR lib/${ANDROID_ABI})
    set(RUNE_VM_LIBRARY_TYPE SHARED)
    set(RUNE_VM_BUILD_SHARED ON)
else()
    set(RUNE_VM_LIBRARY_DIR lib)
    set(RUNE_VM_LIBRARY_TYPE STATIC)
    set(RUNE_VM_BUILD_SHARED OFF)
endif(ANDROID)
    
# Add library
set(RUNE_VM_TARGET_NAME ${PROJECT_NAME})
add_library(${RUNE_VM_TARGET_NAME} ${RUNE_VM_LIBRARY_TYPE} ${SOURCES} ${HEADERS})

target_include_directories(${RUNE_VM_TARGET_NAME} PUBLIC
    ${RUNE_VM_INCLUDES_DIR}
    ${RUNE_VM_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities
    ${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities/delegates
    ${CMAKE_CURRENT_SOURCE_DIR}/src/io)

target_include_directories(
    ${RUNE_VM_TARGET_NAME}
    PRIVATE
    ${RUNE_VM_INFERENCE_TFLITE_INCLUDE_DIRS})

# Link dependencies
target_link_libraries(
    ${RUNE_VM_TARGET_NAME}
    PUBLIC
    fmt::fmt
    nlohmann_json::nlohmann_json
    m3
    mio
    ${RUNE_VM_INFERENCE_TFLITE_LIBRARIES})

# Tests
## gtest
if(RUNE_VM_BUILD_TESTS)
    message(STATUS "RUNE_VM tests are enabled")

    # Include and enable ctest
    include(CTest)
    enable_testing()

    # "Add" csv parser
    set(RUNE_VM_CSV_PARSER_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/extern/fast-cpp-csv-parser)

    # Add gtest
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(BUILD_GMOCK OFF CACHE BOOL "")
    set(INSTALL_GTEST OFF CACHE BOOL "")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/gtest)

    # Add tests directory
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif(RUNE_VM_BUILD_TESTS)

# Examples
if(RUNE_VM_BUILD_EXAMPLES)
    message(STATUS "RUNE_VM examples are enabled")

    # Add examples directory
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/examples)
endif(RUNE_VM_BUILD_EXAMPLES)

# Installation rules
if(RUNE_VM_INSTALL)
    message(STATUS "Installing Rune VM libs")
    install(TARGETS ${RUNE_VM_TARGET_NAME} DESTINATION ${RUNE_VM_LIBRARY_DIR})
    install(DIRECTORY ${RUNE_VM_INCLUDES_DIR}/rune_vm DESTINATION include)
endif(RUNE_VM_INSTALL)
