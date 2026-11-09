# The source compiler helper is also used by slangi builds without an RHI interpreter.
slang_add_target(
    .
    STATIC
    TARGET_NAME slang-hgvm-compiler
    EXCLUDE_FROM_ALL
    EXPLICIT_SOURCE slang-hgvm-compiler.cpp slang-hgvm-compiler.h
    LINK_WITH_PUBLIC core slang
    INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}/include ${slang_SOURCE_DIR}/source
)

if(NOT SLANG_ENABLE_SLANG_RHI)
    return()
endif()

option(
    SLANG_ENABLE_HGVM_EVENT_TRACKING
    "Compile HGVM event instrumentation"
    OFF
)

slang_add_target(
    .
    STATIC
    EXCLUDE_FROM_ALL
    USE_FEWER_WARNINGS
    EXPLICIT_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-runtime.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-texture-blitter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-texture-blitter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-invocation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-events.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-events.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-debug-map.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-debug-map.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-debug.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-debug-session.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-debug-session.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-debug-values.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-resource.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-buffer-pool.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-interpreter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-interpreter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-staged-records.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-staged-record-builder.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-staged-record-builder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-image-io.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgvm-image-io.h
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgbc-validation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-hgbc-validation.h
    LINK_WITH_PUBLIC slang slang-rhi
    LINK_WITH_PRIVATE core miniz
    INCLUDE_DIRECTORIES_PUBLIC
        ${slang_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
    INCLUDE_DIRECTORIES_PRIVATE
        ${slang_SOURCE_DIR}/external/stb
        ${slang_SOURCE_DIR}/external/tinyexr
    INCLUDE_FROM_PRIVATE slang-rhi
)

target_compile_definitions(
    slang-hgvm
    PUBLIC
        SLANG_ENABLE_HGVM_EVENT_TRACKING=$<BOOL:${SLANG_ENABLE_HGVM_EVENT_TRACKING}>
)
