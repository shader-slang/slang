if(SLANG_EMBED_CORE_MODULE)
    set(SLANG_GLSL_MODULE_DEPENDENCY REQUIRES slang-glsl-module)
else()
    set(SLANG_GLSL_MODULE_DEPENDENCY)
endif()

if(SLANG_ENABLE_SLANGC)
    slang_add_target(
        .
        EXECUTABLE
        USE_FEWER_WARNINGS
        DEBUG_DIR ${slang_SOURCE_DIR}
        LINK_WITH_PRIVATE
            core
            slang
            ${SLANG_GLSL_MODULE_DEPENDENCY}
        INCLUDE_DIRECTORIES_PRIVATE
            ${CMAKE_BINARY_DIR}/$<CONFIG>/include
        REQUIRES
            copy_slang_headers
        INSTALL
        EXPORT_SET_NAME SlangTargets
    )
endif()
