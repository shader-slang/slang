---
layout: user-guide
permalink: /user-guide/target-specifics
---

Target specific functionalities
============================

This chapter provides information for target specific functionalities and behaviors.

Target specific Compiler options
--------------------------------

### SPIR-V specific compiler options

The following compiler options are specific to SPIR-V.

-emit-spirv-directly
    Generate SPIR-V output direclty (default)

-emit-spirv-via-glsl
    Generate SPIR-V output by compiling generated GLSL with glslang

-fvk-{b|s|t|u}-shift <N> <space>: For example '-fvk-b-shift <N> <space>' shifts by N the inferred binding
    numbers for all resources in 'b' registers of space <space>. For a resource attached with :register(bX, <space>)
    but not [vk::binding(...)], sets its Vulkan descriptor set to <space> and binding number to X + N. If you need to
    shift the inferred binding numbers for more than one space, provide more than one such option. If more than one
    such option is provided for the same space, the last one takes effect. If you need to shift the inferred binding
    numbers for all sets, use 'all' as <space>.
    * [DXC description](https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#implicit-binding-number-assignment)
    * [GLSL wiki](https://github.com/KhronosGroup/glslang/wiki/HLSL-FAQ#auto-mapped-binding-numbers)

-fvk-bind-globals <N> <descriptor-set>
    Places the $Globals cbuffer at descriptor set <descriptor-set> and binding <N>.
    It lets you specify the descriptor for the source at a certain register.
    * [DXC description](https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#hlsl-global-variables-and-vulkan-binding)

-force-glsl-scalar-layout
-fvk-use-scalar-layout
    Make data accessed through ConstantBuffer, ParameterBlock, StructuredBuffer, ByteAddressBuffer and general pointers follow the 'scalar' layout when targeting GLSL or SPIRV.

-fvk-use-entrypoint-name
    Uses the entrypoint name from the source instead of 'main' in the spirv output.

-fspv-reflect
    Include reflection decorations in the resulting SPIRV for shader parameters.

-fvk-use-gl-layout
    Use std430 layout instead of D3D buffer layout for raw buffer load/stores.

-spirv-core-grammar
    A path to a specific spirv.core.grammar.json to use when generating SPIR-V output


Target specific Attributes 
--------------------------

### SPIR-V specific Attributes

DXC supports a few attributes and command-line arguments for targetting SPIR-V.
You can find a document of how DXC supports [the feature mapping to SPIR-V](https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst).

Similar to DXC, Slang supports a few of the attributes as following:

- [[vk::binding(binding: int, set: int = 0)]]
    Similar to `binding` layout qualifier in Vulkan. It specifies the uniform buffer binding point, and the descriptor set for Vulkan.

- [[vk::location(X)]]
    Same as `location` layout qualifier in Vulkan. For vertex shader inputs, it specifies the number of the vertex attribute from which input values are taken. For inputs of all other shader types, the location specifies a vector number that can be used to match against outputs from a previous shader stage.

- [[vk::index(Y)]]
    Same as `index` layout qualifier in Vulkan. It is valid only when used with [[location(X)]]. For fragment shader outputs, the location and index specify the color output number and index receiving the values of the output. For outputs of all other shader stages, the location specifies a vector number that can be used to match against inputs in a subsequent shader stage.

- [[vk::input_attachment_index(i)]]
    Same as `input_attachment_input` layout qualifier in Vulkan. It selects which subpass input is being read from. It is valid only when used on subpassInput type uniform variables.

- [[vk::push_constant]]
    Same as `push_constant` layout qualifier in Vulkan. It is applicable only to a uniform block and it will be copied to a special memory location where GPU may have a more direct access to.

- [vk::image_format(format : String)]
    Same as `[[vk::image_format("XX")]]` layout qualifier in DXC. Vulkan/GLSL allows the format string to be specified without the keyword, `image_format`.  Consider the following Slang code, as an example,
  ```csharp
  [vk::image_format("r32f")] RWTexture2D<float> typicalTexture;
  ```
  It will generate the following GLSL or SPIR-V code.
  > layout(r32f) uniform image2D typicalTexture_0;
  > %18 = OpTypeImage %float 2D 2 0 0 2 R32f

- [vk::shader_record]
    Same as `shaderRecordEXT` layout qualifier in [GL_EXT_ray_tracing extension](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_ray_tracing.txt).
    It can be used on a buffer block that represents a buffer within a shader record as defined in the Ray Tracing API.

- [vk::aliased_pointer]
    TODO

- [vk_restrict_pointer]
    TODO

- [vk::spirv_instruction(op : int, set : String = "")]
    It is similar to how `__intrinsic_op` works, but it uses the `op-code` of SPIR-V instruction set. You can also specify which instruction set you want to use as a second argument. E.G., `[[vk::spirv_instruction(1, "NonSemantic.DebugBreak")]]`

- [spv_target_env_1_3]
    TODO: I am not sure what it does. There isn't an example on slang-test


Target specific Slang intrinsics
--------------------------------

### SPIR-V specific keywords and intrinsics

Slang doesn't support the following Precision qualifiers in Vulkan. They are simply ignored and all are treated as highp.
 - lowp : RelaxedPrecision, on storage variable and operation
 - mediump : RelaxedPrecision, on storage variable and operation
 - highp : 32-bit, same as int or float

TODO: Cannot think of anything to put here. `spirv_asm` is already well documented in a1-04-interop.md.


Memory pointer
--------------

TODO: Describe any information when using memory pointers for SPIR-V and Metal


Buffer layout and alignment
---------------------------

TODO: Need to digest the comments writting in "compute/buffer-layout.slang" and write up a user-friendly document here.


Matrix majorness (row/column)
-----------------------------

### Matrix majorness for SPIR-V

TODO: Not sure how this is handled.


Summary
-------

This chapter described any specific details for each target.


