function(example dir)
    cmake_parse_arguments(ARG "WIN32_EXECUTABLE" "" "" ${ARGN})

    set(debug_dir ${CMAKE_CURRENT_BINARY_DIR}/${dir})

    # CONFIGURE_DEPENDS removed to prevent intermittent build failures on Windows.
    #
    # Root cause: All examples share the same examples/CMakeFiles/generate.stamp file.
    # When building in parallel, multiple targets trigger simultaneous CMake regenerations,
    # causing a race condition on the shared stamp file (~10% failure rate).
    #
    # Long-term fix: Move each example() call to add_subdirectory() with its own CMakeLists.txt
    # to isolate generate.stamp files. For now, developers must manually re-run cmake configure
    # when adding new asset files to example directories.
    #
    # See: https://gitlab.kitware.com/cmake/cmake/-/issues/21571
    file(
        GLOB asset_files
        "${dir}/*.slang"
        "${dir}/*.jpg"
        "${dir}/*.obj"
        "${dir}/*.mtl"
        "${dir}/*.h"
    )

    list(LENGTH asset_files asset_files_length)
    if(asset_files_length GREATER 0)
        set(copy_assets_target "${dir}-copy-assets")

        add_custom_target(
            ${copy_assets_target}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${debug_dir}
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different ${asset_files}
                ${debug_dir}
            COMMENT "Copy example assets to ${debug_dir}"
        )

        set_target_properties(
            ${copy_assets_target}
            PROPERTIES FOLDER "examples/copy_assets"
        )
    endif()

    # Libraries providing a main function that prints stack traces on exceptions
    if(CMAKE_SYSTEM_NAME MATCHES "Windows")
        # On Windows we have two different versions: main for "console applications" and
        # WinMain for normal Windows applications.
        if(${ARG_WIN32_EXECUTABLE})
            set(main_wrapper_libraries example-winmain)
        else()
            set(main_wrapper_libraries example-main)
        endif()
        # Add stack printing support
        set(main_wrapper_libraries ${main_wrapper_libraries} stacktrace-windows)
        set(main_wrapper_libraries ${main_wrapper_libraries} dbghelp.lib)
    else()
        set(main_wrapper_libraries example-main)
    endif()

    slang_add_target(
        ${dir}
        EXECUTABLE
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE
            core
            example-base
            slang-rhi
            slang
            platform
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
            ${main_wrapper_libraries}
        INCLUDE_DIRECTORIES_PUBLIC ${slang_SOURCE_DIR}
        EXTRA_COMPILE_DEFINITIONS_PRIVATE
            SLANG_EXAMPLE_NAME=${dir}
            $<$<BOOL:${SLANG_ENABLE_XLIB}>:SLANG_ENABLE_XLIB>
        REQUIRED_BY all-examples
        OPTIONAL_REQUIRES ${copy_assets_target} copy-prebuilt-binaries
        FOLDER examples
        DEBUG_DIR ${debug_dir}
        ${ARGN}
    )
endfunction()

if(SLANG_ENABLE_EXAMPLES AND SLANG_ENABLE_SLANG_RHI)
    #
    # Examples
    #
    slang_add_target(
        example-base
        STATIC
        LINK_WITH_PRIVATE
            core
            slang-rhi
            slang
            platform
            stb
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
        FOLDER examples
    )
    slang_add_target(example-main STATIC FOLDER examples)
    slang_add_target(example-winmain STATIC FOLDER examples EXCLUDE_FROM_ALL)
    slang_add_target(stacktrace-windows STATIC FOLDER examples EXCLUDE_FROM_ALL)

    add_custom_target(
        all-examples
        COMMENT "meta target which depends on all examples"
    )
    set_target_properties(all-examples PROPERTIES FOLDER examples)
    example(autodiff-texture WIN32_EXECUTABLE)
    example(cpu-com-example)
    example(cpu-hello-world)
    example(gpu-printing)
    example(hello-world LINK_WITH_PRIVATE Vulkan-Headers)
    example(model-viewer WIN32_EXECUTABLE)
    example(platform-test WIN32_EXECUTABLE)
    example(ray-tracing WIN32_EXECUTABLE)
    example(ray-tracing-pipeline WIN32_EXECUTABLE)
    example(reflection-api)
    example(reflection-parameter-blocks LINK_WITH_PRIVATE Vulkan-Headers)
    example(shader-object)
    example(shader-toy WIN32_EXECUTABLE)
    example(triangle WIN32_EXECUTABLE)
    if(SLANG_ENABLE_AFTERMATH)
        example(nv-aftermath-example WIN32_EXECUTABLE)
    endif()

    if(SLANG_ENABLE_SLANG_RHI)
        example(mlp-training LINK_WITH_PRIVATE slang-rhi)
        example(mlp-training-coopvec LINK_WITH_PRIVATE slang-rhi)
    endif()
endif()
