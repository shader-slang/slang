# Build the compiler-matched provider in a separate CMake process. Importing LLVM14 into
# this project would collide with LLVM21 component targets used by slang-llvm.
option(
    SLANG_ENABLE_NVVM_PROVIDER
    "Build and package the isolated LLVM14 NVVM provider"
    OFF
)
set(SLANG_NVVM_LLVM_DIR
    ""
    CACHE PATH
    "Path to an isolated LLVM 14.0.6 CMake package"
)

if(NOT SLANG_ENABLE_NVVM_PROVIDER)
    return()
endif()
if(CMAKE_CROSSCOMPILING)
    message(
        FATAL_ERROR
        "SLANG_ENABLE_NVVM_PROVIDER currently supports native builds only"
    )
endif()
if(NOT EXISTS "${SLANG_NVVM_LLVM_DIR}/LLVMConfig.cmake")
    message(
        FATAL_ERROR
        "SLANG_ENABLE_NVVM_PROVIDER requires SLANG_NVVM_LLVM_DIR to name an existing isolated LLVM 14.0.6 CMake package"
    )
endif()

include(ExternalProject)

set(_nvvm_binary_dir "${CMAKE_BINARY_DIR}/nvvm-provider/$<CONFIG>")
set(_nvvm_module_suffix "${CMAKE_SHARED_MODULE_SUFFIX}")
if(APPLE)
    # The standalone provider deliberately uses the dynamic-library extension on macOS.
    set(_nvvm_module_suffix ".dylib")
endif()
set(_nvvm_module_name
    "${CMAKE_SHARED_MODULE_PREFIX}slang-llvm-nvvm${_nvvm_module_suffix}"
)
set(_nvvm_module "${_nvvm_binary_dir}/out/${_nvvm_module_name}")
set(_nvvm_cmake_args
    "-DSLANG_SOURCE_DIR:PATH=${slang_SOURCE_DIR}"
    "-DSLANG_NVVM_LLVM_DIR:PATH=${SLANG_NVVM_LLVM_DIR}"
    "-DLLVM_DIR:PATH=${SLANG_NVVM_LLVM_DIR}"
    "-DCMAKE_BUILD_TYPE:STRING=$<CONFIG>"
    "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=<BINARY_DIR>/out"
    "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=<BINARY_DIR>/out"
)
# Each configuration owns a separate child cache. Remove the child's extra configuration
# suffix so the resulting artifact has the same predictable path on single- and multi-config
# generators, including Visual Studio.
foreach(_nvvm_config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${_nvvm_config}" _nvvm_config_upper)
    list(
        APPEND
        _nvvm_cmake_args
        "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_${_nvvm_config_upper}:PATH=<BINARY_DIR>/out"
        "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${_nvvm_config_upper}:PATH=<BINARY_DIR>/out"
    )
endforeach()

# Preserve the native toolchain and target-architecture settings without importing any LLVM
# package from the parent. LIST_SEPARATOR preserves list-valued settings such as macOS arches.
set(_nvvm_forwarded_settings
    CMAKE_C_COMPILER
    CMAKE_CXX_COMPILER
    CMAKE_MAKE_PROGRAM
    CMAKE_TOOLCHAIN_FILE
    CMAKE_GENERATOR_INSTANCE
    CMAKE_CONFIGURATION_TYPES
    CMAKE_SYSROOT
    CMAKE_OSX_ARCHITECTURES
    CMAKE_OSX_SYSROOT
    CMAKE_OSX_DEPLOYMENT_TARGET
    CMAKE_C_COMPILER_TARGET
    CMAKE_CXX_COMPILER_TARGET
    CMAKE_C_COMPILER_EXTERNAL_TOOLCHAIN
    CMAKE_CXX_COMPILER_EXTERNAL_TOOLCHAIN
    CMAKE_C_FLAGS
    CMAKE_CXX_FLAGS
    CMAKE_MODULE_LINKER_FLAGS
)
set(_nvvm_configurations ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE})
list(REMOVE_DUPLICATES _nvvm_configurations)
foreach(_nvvm_config IN LISTS _nvvm_configurations)
    string(TOUPPER "${_nvvm_config}" _nvvm_config)
    list(
        APPEND
        _nvvm_forwarded_settings
        CMAKE_C_FLAGS_${_nvvm_config}
        CMAKE_CXX_FLAGS_${_nvvm_config}
        CMAKE_MODULE_LINKER_FLAGS_${_nvvm_config}
    )
endforeach()
foreach(_nvvm_setting IN LISTS _nvvm_forwarded_settings)
    if(DEFINED ${_nvvm_setting})
        string(REPLACE ";" "|" _nvvm_setting_value "${${_nvvm_setting}}")
        list(
            APPEND
            _nvvm_cmake_args
            "-D${_nvvm_setting}:STRING=${_nvvm_setting_value}"
        )
    endif()
endforeach()

ExternalProject_Add(
    slang-nvvm-provider-build
    SOURCE_DIR "${slang_SOURCE_DIR}/source/slang-llvm-nvvm"
    BINARY_DIR "${_nvvm_binary_dir}"
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
    CMAKE_GENERATOR_PLATFORM "${CMAKE_GENERATOR_PLATFORM}"
    CMAKE_GENERATOR_TOOLSET "${CMAKE_GENERATOR_TOOLSET}"
    LIST_SEPARATOR "|"
    CMAKE_ARGS ${_nvvm_cmake_args}
    BUILD_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR> --config $<CONFIG>
    BUILD_ALWAYS TRUE
    BUILD_BYPRODUCTS "${_nvvm_module}"
    INSTALL_COMMAND ""
    EXCLUDE_FROM_ALL TRUE
)

# ExternalProject's initial directory step does not expand configuration expressions.
# Create the selected configuration directory before its configure command enters it.
ExternalProject_Add_Step(
    slang-nvvm-provider-build
    create-config-directory
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${_nvvm_binary_dir}"
    DEPENDEES patch
    DEPENDERS configure
)

# Always invoke the child build so its dependency scanner notices shared provider ABI headers.
# copy_if_different then leaves staged modules untouched when no provider inputs changed.
add_custom_target(slang-nvvm-provider ALL DEPENDS slang-nvvm-provider-build)
set(_nvvm_have_executable FALSE)
foreach(_nvvm_executable IN ITEMS slangc slang-test test-server)
    if(TARGET ${_nvvm_executable})
        set(_nvvm_have_executable TRUE)
        add_custom_command(
            TARGET slang-nvvm-provider
            POST_BUILD
            COMMAND
                "${CMAKE_COMMAND}" -E make_directory
                "$<TARGET_FILE_DIR:${_nvvm_executable}>"
            COMMAND
                "${CMAKE_COMMAND}" -E copy_if_different "${_nvvm_module}"
                "$<TARGET_FILE_DIR:${_nvvm_executable}>/${_nvvm_module_name}"
            VERBATIM
        )
        add_dependencies(${_nvvm_executable} slang-nvvm-provider)
    endif()
endforeach()
if(NOT _nvvm_have_executable)
    if(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(_nvvm_stage_dir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
        set(_nvvm_stage_dir "${CMAKE_BINARY_DIR}/$<CONFIG>/bin")
    endif()
    add_custom_command(
        TARGET slang-nvvm-provider
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${_nvvm_stage_dir}"
        COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different "${_nvvm_module}"
            "${_nvvm_stage_dir}/${_nvvm_module_name}"
        VERBATIM
    )
endif()
# Match the root Slang executable install layout, which uses bin in SlangTarget.cmake.
install(FILES "${_nvvm_module}" DESTINATION bin COMPONENT slang-llvm-nvvm)
