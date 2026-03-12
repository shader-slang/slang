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
