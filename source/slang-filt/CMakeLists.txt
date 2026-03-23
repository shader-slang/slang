if(SLANG_ENABLE_SLANGFILT)
    slang_add_target(
        .
        EXECUTABLE
        USE_FEWER_WARNINGS
        DEBUG_DIR ${slang_SOURCE_DIR}
        LINK_WITH_PRIVATE
            core
            slang
            Threads::Threads
        INSTALL
        EXPORT_SET_NAME SlangTargets
    )
endif()
