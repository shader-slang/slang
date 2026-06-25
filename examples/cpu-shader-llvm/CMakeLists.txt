example(cpu-shader-llvm TARGET_NAME cpu-shader-llvm-jit EXPLICIT_SOURCE jit.cpp LINK_WITH_PRIVATE stb)

if(SLANG_ENABLE_SLANGC)
    # This example does not set a CMake toolchain for Slang, hence just running
    # it as a custom command.
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shader.o
        COMMAND slangc -target shader-object-code -emit-cpu-via-llvm -O3 -g0 ${CMAKE_CURRENT_SOURCE_DIR}/shader.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shader.o
        COMMAND_EXPAND_LISTS
        MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/shader.slang
        VERBATIM
    )
    add_custom_target(cpu-shader-llvm-link-shader DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/shader.o)
    # This dependency is added so that the custom command above is able to use
    # slangc and slang-llvm.
    add_dependencies(cpu-shader-llvm-link-shader slangc slang-llvm)

    # Mark shader.o as a generated file before `slang_add_target` consumes it
    # via `EXPLICIT_SOURCE` below. `slang_add_target` is a function (separate
    # variable scope from this CMakeLists.txt), and without an explicit
    # GENERATED property the `add_executable` inside the function treats the
    # file as a plain pre-existing input. That gives Ninja only the order-only
    # target-level edge from `REQUIRES cpu-shader-llvm-link-shader`, which under
    # high-parallelism builds (notably slangpy CI building slang as a
    # subproject) races the link step ahead of the slangc invocation that
    # produces shader.o. Setting GENERATED here makes CMake emit a proper
    # file-level Ninja edge from the link's input list to the custom-command
    # producer rule, so the link must wait for shader.o regardless of build
    # scheduling.
    set_source_files_properties(
        ${CMAKE_CURRENT_BINARY_DIR}/shader.o
        PROPERTIES GENERATED TRUE
    )

    # Normally, you would use the below three lines:
    #add_executable(cpu-shader-llvm-link link.cpp ${CMAKE_CURRENT_BINARY_DIR}/shader.o)
    #add_dependencies(cpu-shader-llvm-link cpu-shader-llvm-link-shader)
    #target_link_libraries(cpu-shader-llvm-link PRIVATE stb)

    # However, the Slang project has its own target system which we use to have
    # this example behave like all the others do.
    slang_add_target(
        .
        EXECUTABLE
        TARGET_NAME cpu-shader-llvm-link
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE
            # stb is only used for outputting images. It's not related to using
            # Slang on CPUs.
            stb
        REQUIRED_BY all-examples
        REQUIRES cpu-shader-llvm-link-shader
        EXPLICIT_SOURCE link.cpp ${CMAKE_CURRENT_BINARY_DIR}/shader.o
        FOLDER examples
        DEBUG_DIR ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()
