#
# Helper function to set up a Slang example target
# Handles common boilerplate for all examples: asset copying, main wrapper selection, and target creation
#
function(example dir)
    cmake_parse_arguments(ARG "WIN32_EXECUTABLE" "" "" ${ARGN})

    set(debug_dir ${CMAKE_CURRENT_BINARY_DIR})

    file(
        GLOB asset_files
        CONFIGURE_DEPENDS
        "./*.slang"
        "./*.jpg"
        "./*.obj"
        "./*.mtl"
        "./*.h"
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
        .
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
    # Shared libraries for examples
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

    #
    # Meta target that depends on all examples
    #
    add_custom_target(
        all-examples
        COMMENT "meta target which depends on all examples"
    )
    set_target_properties(all-examples PROPERTIES FOLDER examples)

    #
    # Add individual examples as subdirectories
    #
    add_subdirectory(autodiff-texture)
    add_subdirectory(cpu-com-example)
    add_subdirectory(cpu-hello-world)
    add_subdirectory(gpu-printing)
    add_subdirectory(hello-world)
    add_subdirectory(model-viewer)
    add_subdirectory(platform-test)
    add_subdirectory(ray-tracing)
    add_subdirectory(ray-tracing-pipeline)
    add_subdirectory(reflection-api)
    add_subdirectory(reflection-parameter-blocks)
    add_subdirectory(shader-object)
    add_subdirectory(shader-toy)
    add_subdirectory(triangle)

    #
    # Conditional examples
    #
    if(SLANG_ENABLE_AFTERMATH)
        add_subdirectory(nv-aftermath-example)
    endif()

    if(SLANG_ENABLE_SLANG_RHI)
        add_subdirectory(mlp-training)
        add_subdirectory(mlp-training-coopvec)
    endif()
endif()
