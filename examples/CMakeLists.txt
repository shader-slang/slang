function(example dir)
    set(debug_dir ${CMAKE_CURRENT_BINARY_DIR}/${dir})

    file(
        GLOB asset_files
        CONFIGURE_DEPENDS
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

    slang_add_target(
        ${dir}
        EXECUTABLE
        USE_FEWER_WARNINGS
        LINK_WITH_PRIVATE
            core
            example-base
            slang
            gfx
            gfx-util
            platform
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
        EXTRA_COMPILE_DEFINITIONS_PRIVATE
            $<$<BOOL:${SLANG_ENABLE_XLIB}>:SLANG_ENABLE_XLIB>
        REQUIRED_BY all-examples
        OPTIONAL_REQUIRES ${copy_assets_target} copy-prebuilt-binaries
        FOLDER examples
        DEBUG_DIR ${debug_dir}
        ${ARGN}
    )
endfunction()

if(SLANG_ENABLE_EXAMPLES)
    #
    # Examples
    #
    slang_add_target(
        example-base
        STATIC
        LINK_WITH_PRIVATE
            core
            slang
            gfx
            platform
            stb
            $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
        FOLDER examples
    )

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
    example(shader-object)
    example(shader-toy WIN32_EXECUTABLE)
    example(triangle WIN32_EXECUTABLE)
    if(SLANG_ENABLE_AFTERMATH)
        example(nv-aftermath-example WIN32_EXECUTABLE)
    endif()
endif()
