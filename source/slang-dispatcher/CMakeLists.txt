slang_add_target(
    .
    EXECUTABLE
    OUTPUT_NAME slang
    LINK_WITH_PRIVATE
        core
    INSTALL
    EXPORT_SET_NAME SlangTargets
)

target_compile_definitions(slang-dispatcher PRIVATE "SLANG_VERSION_STRING=\"${PROJECT_VERSION}\"")
