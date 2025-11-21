# model-viewer example

set(debug_dir ${CMAKE_CURRENT_BINARY_DIR})

# Copy example assets
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
    set(copy_assets_target "model-viewer-copy-assets")
    add_custom_target(
        ${copy_assets_target}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${debug_dir}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${asset_files} ${debug_dir}
        COMMENT "Copy example assets to ${debug_dir}"
    )
    set_target_properties(${copy_assets_target} PROPERTIES FOLDER "examples/copy_assets")
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(main_wrapper_libraries example-winmain stacktrace-windows dbghelp.lib)
else()
    set(main_wrapper_libraries example-main)
endif()

slang_add_target(
    .
    EXECUTABLE
    TARGET_NAME model-viewer
    WIN32_EXECUTABLE
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
        SLANG_EXAMPLE_NAME=model-viewer
        $<$<BOOL:${SLANG_ENABLE_XLIB}>:SLANG_ENABLE_XLIB>
    OPTIONAL_REQUIRES ${copy_assets_target} copy-prebuilt-binaries
    FOLDER examples
    DEBUG_DIR ${debug_dir}
)

# Add this example to the all-examples meta target
add_dependencies(all-examples model-viewer)
