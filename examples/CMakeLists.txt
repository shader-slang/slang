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
