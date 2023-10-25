add_library(
  render-test EXCLUDE_FROM_ALL
  diagnostics.cpp
  options.cpp
  png-serialize-util.cpp
  render-test-main.cpp
  shader-input-layout.cpp
  shader-renderer-util.cpp
  slang-support.cpp)
set_default_compile_options(render-test)
target_link_libraries(
  render-test
  core
  compiler-core
  slang
  gfx
  gfx-util
  platform)

if(SLANG_ENABLE_CUDA)
  target_compile_definitions(render-test PRIVATE RENDER_TEST_CUDA)
endif()

if(SLANG_ENABLE_OPTIX)
  target_compile_definitions(render-test PRIVATE RENDER_TEST_OPTIX)
endif()
