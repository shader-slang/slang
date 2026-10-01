slang_add_target(
    .
    STATIC
    EXPORT_MACRO_PREFIX SLANG
    EXCLUDE_FROM_ALL
    USE_EXTRA_WARNINGS
    LINK_WITH_PRIVATE miniz lz4_static Threads::Threads ${CMAKE_DL_LIBS}
    # Public because slang-dictionary.h and slang-hash.h are public headers of
    # this target, and every dependent must agree on which hash map and hash
    # function Slang::Dictionary is built from.
    LINK_WITH_PUBLIC slang-hashmap
    INCLUDE_DIRECTORIES_PUBLIC
        ${slang_SOURCE_DIR}/source
        ${slang_SOURCE_DIR}/include
    PRECOMPILE_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/slang-basic.h
)

if(SLANG_ENABLE_MIMALLOC)
    if(NOT TARGET mimalloc-static)
        message(
            FATAL_ERROR
            "SLANG_ENABLE_MIMALLOC requires the mimalloc-static target"
        )
    endif()
    target_compile_definitions(core PUBLIC SLANG_ENABLE_MIMALLOC=1)
    target_link_libraries(core PUBLIC mimalloc-static)
endif()
