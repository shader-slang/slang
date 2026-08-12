# CMake adds /EHsc to CMAKE_CXX_FLAGS for MSVC, but render-test needs /EHa so that it can catch
# structured exceptions raised by graphics drivers. Replace the option only in this directory so
# that other targets retain their original exception flags.
if(MSVC)
    string(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

slang_add_target(
    ${CMAKE_CURRENT_SOURCE_DIR}
    MODULE
    TARGET_NAME render-test
    EXCLUDE_FROM_ALL
    USE_FEWER_WARNINGS
    LINK_WITH_PRIVATE
        core
        compiler-core
        slang
        slang-rhi
        platform
        stb
        $<$<BOOL:${SLANG_ENABLE_CUDA}>:CUDA::cuda_driver>
    INCLUDE_FROM_PRIVATE slang-rhi
    PRECOMPILE_HEADERS ${SLANG_TOOLS_PCH}
    EXTRA_COMPILE_DEFINITIONS_PRIVATE
        SLANG_SHARED_LIBRARY_TOOL
        $<$<BOOL:${SLANG_ENABLE_CUDA}>:RENDER_TEST_CUDA>
        $<$<BOOL:${SLANG_ENABLE_OPTIX}>:RENDER_TEST_OPTIX>
    EXTRA_COMPILE_OPTIONS_PRIVATE $<$<BOOL:${MSVC}>:/EHa>
    OUTPUT_NAME render-test-tool
    REQUIRED_BY slang-test
    FOLDER test/tools
)

if(SLANG_HAS_WEBGPU_SUPPORT AND CMAKE_SYSTEM_NAME MATCHES "Linux|Darwin")
    # slang-rhi loads Dawn from the directory containing the module/executable that contains RHI.
    # Put render-test beside the runtime sidecar libraries in WGPU builds.
    set_target_properties(
        render-test
        PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY
                "${CMAKE_BINARY_DIR}/$<CONFIG>/${runtime_subdir}"
    )
endif()
