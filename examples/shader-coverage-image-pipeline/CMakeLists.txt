# Vulkan coverage workflow using slang-rhi synthetic resource bindings.
# Build the optional SPIR-V optimizer used by these shaders when available.
example(shader-coverage-image-pipeline OPTIONAL_REQUIRES slang-glslang)
target_include_directories(
    shader-coverage-image-pipeline
    PRIVATE ${slang_SOURCE_DIR}/examples
)
