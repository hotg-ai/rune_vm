cmake_minimum_required(VERSION 3.15)

# Sources & Headers
set(
    SOURCES
)

set(
    HEADERS
)

macro(rune_vm_add_example example_name)
    # Add executable
    set(RUNE_VM_EXAMPLE_TARGET rune_vm_${example_name})
    add_executable(${RUNE_VM_EXAMPLE_TARGET} ${CMAKE_CURRENT_SOURCE_DIR}/${example_name}.cpp ${SOURCES} ${HEADERS})

    # Link dependencies
    target_link_libraries(
        ${RUNE_VM_EXAMPLE_TARGET}
        PRIVATE
        ${RUNE_VM_TARGET_NAME})
endmacro(rune_vm_add_example)

rune_vm_add_example(example0)
