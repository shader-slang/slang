# Slang Examples

This directory contains small example programs showing how to use the Slang language, compiler, and API.

- The [`hello-world`](hello-world/) example shows a minimal example of using Slang shader code more or less like HLSL.

- The [`shader-object`](shader-object/) example shows how Slang's support for interface types can be used to implement shader specialization with simpler logic than preprocessor-based techniques.

- The [`gpu-printing`](gpu-printing/) example shows how Slang's support for string literals can be used to implement a cross-API "GPU `printf`" solution

Most examples use `slang-rhi` to manage GPU resources and dispatch shaders.
Using it is optional: the native coverage backend example below demonstrates
integration with an application's own runtime.

## Choose your coverage integration path

| Your host                             | Start here                                        | Binding demonstrated                                                     |
| ------------------------------------- | ------------------------------------------------- | ------------------------------------------------------------------------ |
| Uses slang-rhi                        | [Image pipeline](shader-coverage-image-pipeline/) | Compiler-assigned placement, registered with RHI before program creation |
| Uses slang-rhi and needs a fixed slot | [BVH traversal](shader-coverage-bvh-traversal/)   | Explicit placement, registered and bound through the same RHI API        |
| Owns its runtime, without slang-rhi   | [Selectable backends](shader-coverage-backends/)  | Native CPU/CUDA/Vulkan/Metal binding from compiler metadata              |
| Loads a precompiled CPU shader        | [CPU tutorial](shader-coverage-tutorial/)         | Standalone host using a sidecar manifest                                 |

Both RHI examples currently run on Vulkan. RHI's synthetic-resource API supports
Vulkan and CUDA; native coverage also works on CPU and Metal. Automatic versus
explicit placement is independent of whether the host uses RHI.
