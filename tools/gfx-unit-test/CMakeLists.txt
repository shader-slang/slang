add_library(
  gfx-unit-test SHARED EXCLUDE_FROM_ALL
  buffer-barrier-test.cpp
  clear-texture-test.cpp
  compute-smoke.cpp
  copy-texture-tests.cpp
  create-buffer-from-handle.cpp
  existing-device-handle-test.cpp
  format-unit-tests.cpp
  get-buffer-resource-handle-test.cpp
  get-cmd-buffer-handle-test.cpp
  get-cmd-queue-handle-test.cpp
  get-supported-resource-states-test.cpp
  get-texture-resource-handle-test.cpp
  gfx-test-texture-util.cpp
  gfx-test-util.cpp
  instanced-draw-tests.cpp
  mutable-shader-object.cpp
  nested-parameter-block.cpp
  ray-tracing-tests.cpp
  resolve-resource-tests.cpp
  root-mutable-shader-object.cpp
  root-shader-parameter.cpp
  sampler-array.cpp
  shader-cache-tests.cpp
  shared-buffers-tests.cpp
  shared-textures-tests.cpp
  swap-chain-resize-test.cpp
  texture-types-tests.cpp)
set_default_compile_options(gfx-unit-test)
target_link_libraries(gfx-unit-test PRIVATE core slang unit-test gfx gfx-util
                                            platform)
target_compile_definitions(gfx-unit-test PRIVATE SLANG_SHARED_LIBRARY_TOOL)
set_target_properties(gfx-unit-test PROPERTIES OUTPUT_NAME gfx-unit-test-tool)
