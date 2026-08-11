if(TARGET slang-glsl-module)
    set(SLANG_GLSL_MODULE_DEPENDENCY REQUIRES slang-glsl-module)
else()
    set(SLANG_GLSL_MODULE_DEPENDENCY)
endif()

if(TARGET generate_core_module_cache)
    set(SLANG_CORE_MODULE_CACHE_DEPENDENCY REQUIRES generate_core_module_cache)
else()
    set(SLANG_CORE_MODULE_CACHE_DEPENDENCY)
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
            Threads::Threads
            ${SLANG_GLSL_MODULE_DEPENDENCY}
            ${SLANG_CORE_MODULE_CACHE_DEPENDENCY}
        PRECOMPILE_HEADERS
            ${slang_SOURCE_DIR}/source/core/slang-basic.h
        INSTALL
        EXPORT_SET_NAME SlangTargets
    )
endif()
