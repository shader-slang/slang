#
# Configure, build and install a CMake project,
#
# For example, LLVM, or Slang itself (for building native generators in a cross
# build)
#
function(build_nested_cmake_project)
    set(no_value_args)
    set(single_value_args
        # The directory in which to find the project's top level CMakeLists.txt
        SOURCE_DIR
        # A directory in which to build the project
        BINARY_DIR
        # The build type, Debug, Release etc..
        BUILD_TYPE
        # Where to place the finished project
        INSTALL_PREFIX
    )
    set(multi_value_args
        # Extra arguments to pass to the CMake configure stage
        EXTRA_CMAKE_ARGS
    )
    cmake_parse_arguments(
        ARG
        "${no_value_args}"
        "${single_value_args}"
        "${multi_value_args}"
        "${ARGN}"
    )
    set(nested_cmake_args
        # General CMake settings, copy settings from the parent project, as
        # they're going to be available
        -G${CMAKE_GENERATOR}
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
        -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}
        -DCMAKE_BUILD_TYPE=${ARG_BUILD_TYPE}
        "${ARG_EXTRA_CMAKE_ARGS}"
    )

    # Configure, build and install a CMake project
    execute_process(
        COMMAND
            ${CMAKE_COMMAND} -S ${ARG_SOURCE_DIR} -B ${ARG_BINARY_DIR}
            ${nested_cmake_args}
        WORKING_DIRECTORY ${ARG_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND
            ${CMAKE_COMMAND} --build ${ARG_BINARY_DIR} -j
            --config=${ARG_BUILD_TYPE}
        WORKING_DIRECTORY ${ARG_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND
            ${CMAKE_COMMAND} --install ${ARG_BINARY_DIR} --prefix
            ${ARG_INSTALL_PREFIX} --config ${ARG_BUILD_TYPE}
        WORKING_DIRECTORY ${ARG_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endfunction()
