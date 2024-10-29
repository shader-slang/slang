if(SLANG_ENABLE_SLANGRT)
    slang_add_target(
        .
        SHARED
        # This compiles 'core' again with the SLANG_RT_DYNAMIC_EXPORT macro defined
        EXTRA_SOURCE_DIRS ${slang_SOURCE_DIR}/source/core
        USE_EXTRA_WARNINGS
        LINK_WITH_PRIVATE miniz lz4_static Threads::Threads ${CMAKE_DL_LIBS}
        LINK_WITH_PUBLIC unordered_dense::unordered_dense
        EXPORT_MACRO_PREFIX SLANG_RT
        INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}/include
        INSTALL
    )
endif()
