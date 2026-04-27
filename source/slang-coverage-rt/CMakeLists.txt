if(SLANG_ENABLE_SHADER_COVERAGE_RT)
    slang_add_target(
        .
        STATIC
        USE_EXTRA_WARNINGS
        INCLUDE_DIRECTORIES_PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            # Public so consumers of slang-coverage.h transitively pick up
            # slang.h (the header includes it for SlangResult / SLANG_E_*).
            # Customers integrating with Slang already have this path on
            # their include line; this is just for the in-tree build.
            ${slang_SOURCE_DIR}/include
    )
endif()
