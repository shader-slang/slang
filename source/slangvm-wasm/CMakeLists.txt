# The browser runtime links the compiler and HGVM into one module so native type
# identities, invocation, and ownership have the same implementation as other hosts.
if(NOT EMSCRIPTEN OR NOT SLANG_ENABLE_SLANG_RHI)
    return()
endif()

slang_add_target(
    .
    EXECUTABLE
    EXCLUDE_FROM_ALL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE
        slang-hgvm
        slang-hgvm-compiler
        slang
        slang-rhi
        core
        compiler-core
        miniz
        lz4_static
        slang-capability-defs
        slang-capability-lookup
        slang-fiddle-output
        slang-lookup-tables
    INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}/include ${slang_SOURCE_DIR}/source
)

set(slangvm_package_dir "${slang_BINARY_DIR}/$<CONFIG>/slangvm")
set_target_properties(
    slangvm-wasm
    PROPERTIES
        SUFFIX ".js"
        RUNTIME_OUTPUT_DIRECTORY "${slangvm_package_dir}/wasm"
)
# Explicit per-configuration output directories also override caller-supplied
# CMAKE_RUNTIME_OUTPUT_DIRECTORY values applied by slang_add_target.
foreach(config Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${config}" config_upper)
    set_property(
        TARGET slangvm-wasm
        PROPERTY RUNTIME_OUTPUT_DIRECTORY_${config_upper}
            "${slang_BINARY_DIR}/${config}/slangvm/wasm"
    )
endforeach()

target_compile_options(slangvm-wasm PRIVATE "--use-port=emdawnwebgpu")
target_link_options(
    slangvm-wasm
    PRIVATE
        "--bind"
        "--no-entry"
        "--use-port=emdawnwebgpu"
        "-sJSPI=1"
        "-sEXPORT_ES6=1"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=createSlangVMModule"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sSTACK_SIZE=16777216"
)

include(package-notices.cmake)

# Keep package assembly independent of npm so both plain browser import maps and
# JavaScript bundlers consume exactly the same Slang-owned runtime artifacts.
add_custom_target(
    slangvm-javascript
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${slang_SOURCE_DIR}/javascript/slangvm/src" "${slangvm_package_dir}/src"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${slang_BINARY_DIR}/source/standard-modules/host/source" "${slangvm_package_dir}/slang"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${slang_BINARY_DIR}/source/standard-modules/numerics/source" "${slangvm_package_dir}/slang"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${slang_SOURCE_DIR}/LICENSES" "${slangvm_package_dir}/LICENSES"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${slang_SOURCE_DIR}/javascript/slangvm/package.json"
        "${slang_SOURCE_DIR}/javascript/slangvm/README.md"
        "${slang_SOURCE_DIR}/LICENSE"
        "${slangvm_package_dir}"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${slangvm_package_dir}/third-party-notices"
    ${slangvm_notice_copy_commands}
    DEPENDS slangvm-wasm
    VERBATIM
)

install(
    DIRECTORY
        "${slang_SOURCE_DIR}/javascript/slangvm/src"
        "${slang_SOURCE_DIR}/LICENSES"
    DESTINATION javascript/slangvm
    COMPONENT slangvm-javascript
    EXCLUDE_FROM_ALL
)
install(
    FILES
        "${slang_SOURCE_DIR}/javascript/slangvm/package.json"
        "${slang_SOURCE_DIR}/javascript/slangvm/README.md"
        "${slang_SOURCE_DIR}/LICENSE"
    DESTINATION javascript/slangvm
    COMPONENT slangvm-javascript
    EXCLUDE_FROM_ALL
)
install(
    FILES
        "$<TARGET_FILE:slangvm-wasm>"
        "$<TARGET_FILE_DIR:slangvm-wasm>/slangvm-wasm.wasm"
    DESTINATION javascript/slangvm/wasm
    COMPONENT slangvm-javascript
    EXCLUDE_FROM_ALL
)
