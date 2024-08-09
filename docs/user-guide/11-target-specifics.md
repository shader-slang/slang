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

-force-glsl-scalar-layout
-fvk-use-scalar-layout
    Make data accessed through ConstantBuffer, ParameterBlock, StructuredBuffer, ByteAddressBuffer and general pointers follow the 'scalar' layout when targeting GLSL or SPIRV.

-fvk-use-entrypoint-name
    Uses the entrypoint name from the source instead of 'main' in the spirv output.

-fspv-reflect
    Include reflection decorations in the resulting SPIRV for shader parameters.

-fvk-bind-globals
    <N> <descriptor-set>: Places the $Globals cbuffer at descriptor set <descriptor-set> and binding <N>.

-fvk-use-gl-layout
    Use std430 layout instead of D3D buffer layout for raw buffer load/stores.

-spirv-core-grammar
    A path to a specific spirv.core.grammar.json to use when generating SPIR-V output


Target specific Attributes 
--------------------------

### SPIR-V specific Attributes

The following attributes are specific to SPIR-V.

[vk_binding(binding: int, set: int = 0)]

[vk_shader_record]

[vk_push_constant]

[vk_location(location : int)]

[vk_index(index : int)]

[vk_spirv_instruction(op : int, set : String = "")]

[vk_input_attachment_index(location : int)]

[spv_target_env_1_3]

[vk_image_format(format : String)]


Target specific Slang intrinsics
--------------------------------

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


