# ---- Slang integration tests -------------------------------------------------

find_program(SLANGC slangc
    HINTS
        $ENV{SLANG_DIR}/bin
)

if (NOT SLANGC)
    message(STATUS "slangc not found — skipping slang integration tests")
    return()
endif()

message(STATUS "Found slangc: ${SLANGC}")

# ---- Compile .slang sources to SPIR-V ----------------------------------------

set(SLANG_SRC_DIR  ${CMAKE_CURRENT_SOURCE_DIR})
set(SLANG_SPV_DIR  ${CMAKE_CURRENT_BINARY_DIR}/spv)
file(MAKE_DIRECTORY ${SLANG_SPV_DIR})

set(SLANG_SOURCES
    ${SLANG_SRC_DIR}/add_buffers.slang
    ${SLANG_SRC_DIR}/inlined_func.slang
)

set(SLANG_SPV_BINARIES "")
foreach(SLANG_FILE ${SLANG_SOURCES})
    get_filename_component(STEM ${SLANG_FILE} NAME_WE)
    set(SPV_FILE ${SLANG_SPV_DIR}/${STEM}.spv)
    list(APPEND SLANG_SPV_BINARIES ${SPV_FILE})
    add_custom_command(
        OUTPUT  ${SPV_FILE}
        COMMAND ${SLANGC} -target spirv -g2 -o ${SPV_FILE} ${SLANG_FILE}
        DEPENDS ${SLANG_FILE}
        COMMENT "Compiling ${STEM}.slang with slangc"
    )
endforeach()

add_custom_target(spvdb_slang_spirv ALL DEPENDS ${SLANG_SPV_BINARIES})

# ---- Test executable ---------------------------------------------------------

add_executable(spvdb_slang_tests
    test_slang.cpp
)

add_dependencies(spvdb_slang_tests spvdb_slang_spirv)

target_link_libraries(spvdb_slang_tests PRIVATE spvdb Catch2::Catch2WithMain)

target_compile_definitions(spvdb_slang_tests PRIVATE
    SLANG_TEST_SPV_DIR="${SLANG_SPV_DIR}"
    SLANG_TEST_SRC_DIR="${SLANG_SRC_DIR}"
)

include(Catch)
catch_discover_tests(spvdb_slang_tests)
