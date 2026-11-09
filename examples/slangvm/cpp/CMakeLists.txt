# Use the native HGVM libraries directly; this example does not use the Python extension.
slang_add_target(
    .
    EXECUTABLE
    TARGET_NAME slangvm-hello-world
    EXPLICIT_SOURCE hello-world.cpp
    LINK_WITH_PRIVATE core slang-hgvm slang-hgvm-compiler
    REQUIRED_BY all-examples
    FOLDER examples
    DEBUG_DIR ${slang_SOURCE_DIR}
)

set_target_properties(
    slangvm-hello-world
    PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS "examples/slangvm/cpp/hello-world.slang"
)
