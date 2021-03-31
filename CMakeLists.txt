cmake_minimum_required(VERSION 3.15)
project(rune_vm)

# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Werror=return-type")

# Options
option(RUNE_VM_INSTALL "To install or not to install. Might be useful if you add this library as submodule dep" ON)

# Add external dependencies
## wasm3-c
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/wasm3/source ${CMAKE_CURRENT_BINARY_DIR}/extern/wasm3_m3)
## fmt
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/fmt ${CMAKE_CURRENT_BINARY_DIR}/extern/fmt)
## mio
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/mio ${CMAKE_CURRENT_BINARY_DIR}/extern/mio)

# Add internal subdirectories
set(RUNE_VM_INCLUDES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(RUNE_VM_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
# # io (formatting)
set(RUNE_VM_IO_TARGET rune_vm_io)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/io)
# # wasm_backends
set(RUNE_VM_BACKEND_TARGET rune_vm_backend)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/wasm_backends)
# # capabilities
set(RUNE_VM_CAPABILITIES_TARGET rune_vm_capabilities)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src/capabilities)

# Sources & Headers
set(
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RuneVm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HostFunctions.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RuneInterop.cpp
)
set(
    HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/RuneVm.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/Log.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/Capabilities.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/rune_vm/VirtualInterface.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HostFunctions.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RuneInterop.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Common.hpp
)

# Add library
set(RUNE_VM_TARGET_NAME ${PROJECT_NAME})
add_library(${RUNE_VM_TARGET_NAME} STATIC ${SOURCES} ${HEADERS})

# Link dependencies
target_link_libraries(
    ${RUNE_VM_TARGET_NAME}
    PRIVATE
    fmt::fmt
    ${RUNE_VM_BACKEND_TARGET}
    ${RUNE_VM_CAPABILITIES_TARGET})

# Installation rules
if(RUNE_VM_INSTALL)
    install(TARGETS ${RUNE_VM_TARGET_NAME} DESTINATION lib)
	install(DIRECTORY ${RUNE_VM_INCLUDES_DIR}/rune_vm DESTINATION include)
endif(RUNE_VM_INSTALL)
