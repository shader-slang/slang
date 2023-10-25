add_library(
  render-test SHARED EXCLUDE_FROM_ALL
  diagnostics.cpp
  options.cpp
  png-serialize-util.cpp
  render-test-main.cpp
  shader-input-layout.cpp
  shader-renderer-util.cpp
  slang-support.cpp)
set_default_compile_options(render-test)
target_link_libraries(render-test PRIVATE core compiler-core slang gfx gfx-util
                                          platform)
target_compile_definitions(render-test PRIVATE SLANG_SHARED_LIBRARY_TOOL)

set_target_properties(render-test PROPERTIES OUTPUT_NAME render-test-tool)

if(SLANG_ENABLE_CUDA AND CUDAToolkit_FOUND)
  target_link_libraries(render-test PRIVATE CUDA::cuda_driver CUDA::cudart)
  target_compile_definitions(render-test PRIVATE RENDER_TEST_CUDA)
endif()

if(SLANG_ENABLE_OPTIX)
  target_compile_definitions(render-test PRIVATE RENDER_TEST_OPTIX)
endif()
