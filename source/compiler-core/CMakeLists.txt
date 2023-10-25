set(SLANG_COMPILER_CORE_SOURCE
    slang-artifact-associated-impl.cpp
    slang-artifact-container-util.cpp
    slang-artifact-desc-util.cpp
    slang-artifact-diagnostic-util.cpp
    slang-artifact-handler-impl.cpp
    slang-artifact-helper.cpp
    slang-artifact-impl.cpp
    slang-artifact-representation-impl.cpp
    slang-artifact-util.cpp
    slang-command-line-args.cpp
    slang-core-diagnostics.cpp
    slang-diagnostic-sink.cpp
    slang-doc-extractor.cpp
    slang-downstream-compiler-set.cpp
    slang-downstream-compiler-util.cpp
    slang-downstream-compiler.cpp
    slang-dxc-compiler.cpp
    slang-fxc-compiler.cpp
    slang-gcc-compiler-util.cpp
    slang-glslang-compiler.cpp
    slang-include-system.cpp
    slang-json-diagnostics.cpp
    slang-json-lexer.cpp
    slang-json-native.cpp
    slang-json-parser.cpp
    slang-json-rpc-connection.cpp
    slang-json-rpc.cpp
    slang-json-source-map-util.cpp
    slang-json-value.cpp
    slang-language-server-protocol.cpp
    slang-lexer.cpp
    slang-llvm-compiler.cpp
    slang-name-convention-util.cpp
    slang-name.cpp
    slang-nvrtc-compiler.cpp
    slang-perfect-hash.cpp
    slang-slice-allocator.cpp
    slang-source-embed-util.cpp
    slang-source-loc.cpp
    slang-source-map.cpp
    slang-spirv-core-grammar.cpp
    slang-test-server-protocol.cpp
    slang-token.cpp
    slang-visual-studio-compiler-util.cpp)

if(WIN32)
  list(APPEND SLANG_COMPILER_CORE_SOURCE
       windows/slang-win-visual-studio-util.cpp)
endif()

if(UNIX)
  # Nothing yet
  list(APPEND SLANG_COMPILER_CORE_SOURCE)
endif()

if(NOT MSVC)
  # This is necessary to compile the DXC headers
  set_source_files_properties(slang-dxc-compiler.cpp
                              PROPERTIES COMPILE_FLAGS "-fms-extensions")
endif()

add_library(compiler-core STATIC EXCLUDE_FROM_ALL ${SLANG_COMPILER_CORE_SOURCE})
set_default_compile_options(compiler-core USE_EXTRA_WARNINGS)
target_link_libraries(compiler-core PRIVATE core)
