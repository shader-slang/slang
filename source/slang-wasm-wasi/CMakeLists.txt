#
# slang-wasm-wasi — WASI reactor WebAssembly module
#
# Only built under the 'wasi' preset (wasi-sdk). No main(); exports _initialize
# and a flat C ABI shim callable from any WASI-compatible runtime.
# See source/slang-wasm-wasi/README.md for the build sequence and C ABI reference.
#

if(NOT CMAKE_SYSTEM_NAME STREQUAL "WASI")
    return()
endif()

#
# Regenerate enum bindings when slang.h changes
#
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(_slang_h "${slang_SOURCE_DIR}/include/slang.h")
set(_generator_script
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate-slang-bindings.py"
)
set(_enum_meta_cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/slang-wasm-wasi-enum-generated.cpp"
)

# `cmake --build` reruns the generator whenever slang.h or the
# generator script itself changes, without requiring a fresh `cmake`
# reconfigure first.
add_custom_command(
    OUTPUT "${_enum_meta_cpp}"
    COMMAND
        "${Python3_EXECUTABLE}" "${_generator_script}" "--slang-h" "${_slang_h}"
        "--cpp-out" "${_enum_meta_cpp}"
    DEPENDS "${_slang_h}" "${_generator_script}"
    COMMENT "slang-wasm-wasi: regenerating enum bindings from slang.h"
    VERBATIM
)

# libunwind is not auto-linked by -fwasm-exceptions in wasi-sdk; the eh/
# sysroot subdirectory holds the EH-aware variant.
set(WASI_EH_LIB_DIR "${CMAKE_SYSROOT}/lib/wasm32-wasip1/eh")

add_executable(
    slang-wasm-wasi
    slang-wasm-wasi.cpp
    slang-wasm-wasi-enum-generated.cpp
)

set_target_properties(
    slang-wasm-wasi
    PROPERTIES
        SUFFIX ".wasm"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release/bin"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/Debug/bin"
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
            "${CMAKE_BINARY_DIR}/RelWithDebInfo/bin"
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/MinSizeRel/bin"
)

target_include_directories(slang-wasm-wasi PRIVATE ${slang_SOURCE_DIR}/include)

target_link_libraries(
    slang-wasm-wasi
    PRIVATE
        slang
        core
        compiler-core
        slang-capability-defs
        slang-capability-lookup
        slang-fiddle-output
        slang-lookup-tables
        miniz
        lz4_static
)

target_link_options(
    slang-wasm-wasi
    PRIVATE
        # Strip DWARF, matching emcc (which strips at link by default; wasm-ld
        # does not). The static libs are built with -g in Release, so their
        # debug info would otherwise bloat the .wasm from ~22 MB to ~138 MB.
        -Wl,--strip-debug
        # Reactor: link crt1-reactor.o which provides _initialize (calls
        # __wasm_call_ctors) and emits no WASM start section.
        -mexec-model=reactor
        # Large stack: Slang's AST/IR passes recurse deeply. 8 MB avoids stack traps.
        -Wl,-z,stack-size=8388608
        # Export the C++ exception tag so the EH runtime is reachable from the host.
        -Wl,--export=__cpp_exception
        # Export the reactor init entry point.
        -Wl,--export=_initialize
        # Export the flat C ABI shim and allocator symbols.
        -Wl,--export=slang_wasm_alloc
        -Wl,--export=slang_wasm_free
        -Wl,--export=slang_wasm_session_create
        -Wl,--export=slang_wasm_session_destroy
        -Wl,--export=slang_wasm_compile
        # module handle exports.
        -Wl,--export=slang_wasm_session_load_module
        -Wl,--export=slang_wasm_module_destroy
        -Wl,--export=slang_wasm_module_entry_point_count
        -Wl,--export=slang_wasm_module_entry_point_name_ptr
        -Wl,--export=slang_wasm_module_entry_point_name_len
        -Wl,--export=slang_wasm_compile_entry_point
        -Wl,--export=slang_wasm_compile_module
        # module serialisation exports.
        -Wl,--export=slang_wasm_module_serialize
        -Wl,--export=slang_wasm_session_load_module_ir
        # type conformance exports.
        -Wl,--export=slang_wasm_type_conformances_create
        -Wl,--export=slang_wasm_type_conformances_add
        -Wl,--export=slang_wasm_type_conformances_destroy
        # specialization exports.
        -Wl,--export=slang_wasm_spec_args_create
        -Wl,--export=slang_wasm_spec_args_add_type
        -Wl,--export=slang_wasm_spec_args_add_expr
        -Wl,--export=slang_wasm_spec_args_destroy
        -Wl,--export=slang_wasm_compile_specialized_entry_point
        # declaration reflection exports.
        -Wl,--export=slang_wasm_module_decl_reflection_json
        # disassembly export.
        -Wl,--export=slang_wasm_module_disassemble
        -Wl,--export=slang_wasm_result_succeeded
        -Wl,--export=slang_wasm_result_code_ptr
        -Wl,--export=slang_wasm_result_code_len
        -Wl,--export=slang_wasm_result_reflection_json_ptr
        -Wl,--export=slang_wasm_result_reflection_json_len
        -Wl,--export=slang_wasm_result_diagnostics_ptr
        -Wl,--export=slang_wasm_result_diagnostics_len
        -Wl,--export=slang_wasm_result_destroy
        -Wl,--export=slang_wasm_version
        # enum metadata exports (static JSON blob, no free required).
        -Wl,--export=slang_wasm_enum_metadata_ptr
        -Wl,--export=slang_wasm_enum_metadata_len
        -Wl,--export=slang_wasm_target_from_string
        -Wl,--export=slang_wasm_stage_from_string
        # session descriptor builder exports.
        -Wl,--export=slang_wasm_target_list_create
        -Wl,--export=slang_wasm_target_list_add
        -Wl,--export=slang_wasm_target_list_destroy
        -Wl,--export=slang_wasm_macro_list_create
        -Wl,--export=slang_wasm_macro_list_add
        -Wl,--export=slang_wasm_macro_list_destroy
        -Wl,--export=slang_wasm_path_list_create
        -Wl,--export=slang_wasm_path_list_add
        -Wl,--export=slang_wasm_path_list_destroy
        -Wl,--export=slang_wasm_options_create
        -Wl,--export=slang_wasm_options_add_string
        -Wl,--export=slang_wasm_options_add_int
        -Wl,--export=slang_wasm_options_destroy
        -Wl,--export=slang_wasm_session_create2
        # libunwind is not auto-linked by -fwasm-exceptions in wasi-sdk.
        -L${WASI_EH_LIB_DIR}
        -lunwind
)
