---
layout: user-guide
permalink: /user-guide/target-specifics
---

SPIR-V specific functionalities
===============================

This chapter provides information for SPIR-V specific functionalities and behaviors.

Experimental support for the older versions of SPIR-V
-----------------------------------------------------

Support for SPIR-V 1.0, 1.1 and 1.2 is still experimental. When targeting the older SPIR-V profiles, Slang may produce SPIR-V that uses the instructions and keywords that were introduced in the later versions of SPIR-V.


Combined texture sampler
------------------------
Slang supports Combined texture sampler such as `Sampler2D`.
Slang emits SPIR-V code with `OpTypeSampledImage` instruction.

You can specify two different register numbers for each: one for the texture register and another for the sampler register.
```
Sampler2D explicitBindingSampler : register(t4): register(s3);
```


Behavior of `discard` after SPIR-V 1.6
--------------------------------------

`discard` is translated to OpKill in SPIR-V 1.5 and earlier. But it is translated to OpDemoteToHelperInvocation in SPIR-V 1.6.
You can use OpDemoteToHelperInvocation by explicitly specifying the capability, "SPV_EXT_demote_to_helper_invocation".

As an example, the following command-line arguments can control the behavior of `discard` when targeting SPIR-V.
```
slangc.exe test.slang -target spirv -profile spirv_1_5 # emits OpKill 
slangc.exe test.slang -target spirv -profile spirv_1_6 # emits OpDemoteToHelperInvocation 
slangc.exe test.slang -target spirv -capability SPV_EXT_demote_to_helper_invocation -profile spirv_1_5 # emits OpDemoteToHelperInvocation 
```


Support HLSL features when targeting SPIR-V
--------------------------------------------

Slang supports the following HLSL feature sets when targeting SPIR-V.
 - ray tracing,
 - inline ray tracing,
 - mesh shader,
 - tessellation shader,
 - geometry shader,
 - wave intrinsics,
 - barriers,
 - atomics,
 - and more


Unsupported HLSL features when targeting SPIR-V
-----------------------------------------------

Due to the limitations and differences on SPIR-V, there are [HLSL features known to be unsupported](https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#unsupported-hlsl-features) when targeting SPIR-V.


Unsupported GLSL keywords when targeting SPIR-V
-----------------------------------------------

Slang doesn't support the following Precision qualifiers in Vulkan.
 - lowp : RelaxedPrecision, on storage variable and operation
 - mediump : RelaxedPrecision, on storage variable and operation
 - highp : 32-bit, same as int or float

Slang ignores the keywords above and all of them are treated as `highp`.


Supported atomic types for each target
--------------------------------------
Shader Model 6.2 introduced [16-bit scalar types](https://github.com/microsoft/DirectXShaderCompiler/wiki/16-Bit-Scalar-Types) such as float16 and int16_t, but they didn't come with any atomic operations.
Shader Model 6.6 introduced [atomic operations for 64-bit integer types and bitwise atomic operations for 32-bit float type](https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Int64_and_Float_Atomics.html), but 16-bit integer types and 16-bit float types are not a part of it.

[GLSL 4.3](https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.30.pdf) introduced atomic operations for 32-bit integer types.
GLSL 4.4 with [GL_EXT_shader_atomic_int64](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GL_EXT_shader_atomic_int64.txt) can use atomic operations for 64-bit integer types.
GLSL 4.6 with [GLSL_EXT_shader_atomic_float](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_shader_atomic_float.txt) can use atomic operations for 32-bit float type.
GLSL 4.6 with [GLSL_EXT_shader_atomic_float2](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_shader_atomic_float2.txt) can use atomic operations for 16-bit float type.

SPIR-V 1.5 with [SPV_EXT_shader_atomic_float_add](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/EXT/SPV_EXT_shader_atomic_float_add.asciidoc) and [SPV_EXT_shader_atomic_float_min_max](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/EXT/SPV_EXT_shader_atomic_float_min_max.asciidoc) can use atomic operations for 32-bit float type and 64-bit float type.
SPIR-V 1.5 with [SPV_EXT_shader_atomic_float16_add](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/extensions/EXT/SPV_EXT_shader_atomic_float16_add.asciidoc) can use atomic operations for 16-bit float type

+--------+-----------------+-----------------+-----------------------+------------------+------------------+
|        |  32-bit integer | 64-bit integer  |      32-bit float     |  64-bit float    |   16-bit float   |
+--------+-----------------+-----------------+-----------------------+------------------+------------------+
| HLSL   |   Yes (SM5.0)   |   Yes (SM6.6)   | Only bit-wise (SM6.6) |       No         |      No
| GLSL   |   Yes (GL4.3)   | Yes (GL4.4+ext) |    Yes (GL4.6+ext)    | Yes (GL4.6+ext)  | Yes (GL4.6+ext)
| SPIR-V |   Yes           |     Yes         |    Yes (SPV1.5+ext)   | Yes (SPV1.5+ext) | Yes (SPV1.5+ext)
+--------+-----------------+-----------------+-----------------------+------------------+------------------+


ConstantBuffer, (RW)StructuredBuffer (RW)ByteAddressBuffer
----------------------------------------------------------

Each member in a `ConstantBuffer` will be emitted as `uniform` parameter.
StructuredBuffer and ByteAddressBuffer are translated to a shader storage buffer with `readonly` layout.
RWStructuredBuffer and RWByteAddressBuffer are translated to a shader storage buffer with `read-write` layout.

If you need to apply a different buffer layout for indivisual StructuredBuffer, you can specify the layout as a second generic argument to StructuredBuffer. E.g., StructuredBuffer<T, Std140Layout>, StructuredBuffer<T, Std430Layout> or StructuredBuffer<T, ScalarLayout>.

Note that there are compiler options, "-fvk-use-scalar-layout" and "-force-glsl-scalar-layout".
These options do the same but they are applied globally.

ParameterBlock for SPIR-V target
--------------------------------

`ParameterBlock` is a Slang generic type for binding uniform parameters.
It is similar to `ConstantBuffer` in HLSL, and `ParameterBlock` can include not only constant parameters but also descriptors such as Texture2D or StructuredBuffer.

`ParameterBlock` is designed specifically for d3d/vulkan/metal, so that parameters are laid out more naturally on these platforms. For Vulkan, when a ParameterBlock doesn't contain nested parameter block fields, it always maps to a single descriptor set, with a dedicated set number and every resources is placed into the set with binding index starting from 0.

When both ordinary data fields and resource typed fields exist in a parameter block, all ordinary data fields will be grouped together into a uniform buffer and appear as a binding 0 of the resulting descriptor set.


SPIR-V specific Compiler options
--------------------------------

The following compiler options are specific to SPIR-V.

### -emit-spirv-directly
Generate SPIR-V output directly (default)
It cannot be used with -emit-spirv-via-glsl

### -emit-spirv-via-glsl
Generate SPIR-V output by compiling to glsl source first, then use glslang compiler to produce SPIRV from the glsl.
It cannot be used with -emit-spirv-directly

### -g
Include debug information in the generated code, where possible.
When targeting SPIR-V, this option emits [SPIR-V NonSemantic Shader DebugInfo Instructions](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/nonsemantic/NonSemantic.Shader.DebugInfo.100.asciidoc).

### -O<optimization-level>
Set the optimization level.
Under `-O0` option, Slang will not perform extensive inlining for all functions calls, instead it will preserve the call graph as much as possible to help with understanding the SPIRV structure and diagnosing any downstream toolchain issues.

### -fvk-{b|s|t|u}-shift <N> <space>
For example '-fvk-b-shift <N> <space>' shifts by N the inferred binding
numbers for all resources in 'b' registers of space <space>. For a resource attached with :register(bX, <space>)
but not [vk::binding(...)], sets its Vulkan descriptor set to <space> and binding number to X + N. If you need to
shift the inferred binding numbers for more than one space, provide more than one such option. If more than one
such option is provided for the same space, the last one takes effect. If you need to shift the inferred binding
numbers for all sets, use 'all' as <space>.
* [DXC description](https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#implicit-binding-number-assignment)
* [GLSL wiki](https://github.com/KhronosGroup/glslang/wiki/HLSL-FAQ#auto-mapped-binding-numbers)

### -fvk-bind-globals <N> <descriptor-set>
Places the $Globals cbuffer at descriptor set <descriptor-set> and binding <N>.
It lets you specify the descriptor for the source at a certain register.
* [DXC description](https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#hlsl-global-variables-and-vulkan-binding)

### -fvk-use-scalar-layout, -force-glsl-scalar-layout
Make data accessed through ConstantBuffer, ParameterBlock, StructuredBuffer, ByteAddressBuffer and general pointers follow the 'scalar' layout when targeting GLSL or SPIRV.

### -fvk-use-gl-layout
Use std430 layout instead of D3D buffer layout for raw buffer load/stores.

### -fvk-use-entrypoint-name
Uses the entrypoint name from the source instead of 'main' in the spirv output.

### -fspv-reflect
Include reflection decorations in the resulting SPIRV for shader parameters.

### -spirv-core-grammar
A path to a specific spirv.core.grammar.json to use when generating SPIR-V output


SPIR-V specific Attributes 
--------------------------

DXC supports a few attributes and command-line arguments for targeting SPIR-V.
You can find a document of how DXC supports [the feature mapping to SPIR-V](https://github.com/microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst).

Similar to DXC, Slang supports a few of the attributes as following:

### [[vk::binding(binding: int, set: int = 0)]]
Similar to `binding` layout qualifier in Vulkan. It specifies the uniform buffer binding point, and the descriptor set for Vulkan.

### [[vk::location(X)]]
Same as `location` layout qualifier in Vulkan. For vertex shader inputs, it specifies the number of the vertex attribute from which input values are taken. For inputs of all other shader types, the location specifies a vector number that can be used to match against outputs from a previous shader stage.

### [[vk::index(Y)]]
Same as `index` layout qualifier in Vulkan. It is valid only when used with [[location(X)]]. For fragment shader outputs, the location and index specify the color output number and index receiving the values of the output. For outputs of all other shader stages, the location specifies a vector number that can be used to match against inputs in a subsequent shader stage.

### [[vk::input_attachment_index(i)]]
Same as `input_attachment_index` layout qualifier in Vulkan. It selects which subpass input is being read from. It is valid only when used on subpassInput type uniform variables.

### [[vk::push_constant]]
Same as `push_constant` layout qualifier in Vulkan. It is applicable only to a uniform block and it will be copied to a special memory location where GPU may have a more direct access to.

### [vk::image_format(format : String)]
Same as `[[vk::image_format("XX")]]` layout qualifier in DXC. Vulkan/GLSL allows the format string to be specified without the keyword, `image_format`.  Consider the following Slang code, as an example,
```csharp
[vk::image_format("r32f")] RWTexture2D<float> typicalTexture;
```
It will generate the following GLSL or SPIR-V code.
> layout(r32f) uniform image2D typicalTexture_0;
> %18 = OpTypeImage %float 2D 2 0 0 2 R32f

### [vk::shader_record]
Same as `shaderRecordEXT` layout qualifier in [GL_EXT_ray_tracing extension](https://github.com/KhronosGroup/GLSL/blob/main/extensions/ext/GLSL_EXT_ray_tracing.txt).
It can be used on a buffer block that represents a buffer within a shader record as defined in the Ray Tracing API.


Multiple entry points support
-----------------------------

To use multiple entry points, you will need to use a compiler option, `-fvk-use-entrypoint-name`.

Because GLSL requires the entry point to be named, "main", a GLSL shader can have only one entry point.
The default behavior of Slang is to rename all entry points to "main" when targeting SPIR-V.

When there are more than one entry point, the default behavior will prevent a shader from having more than one entry point.
To generate a valid SPIR-V with multiple entry points, use `-fvk-use-entrypoint-name` compiler option to disable the renaming behavior and preserve the entry point names.


Memory pointer is experimental
------------------------------

Slang supports memory pointers when targetting SPIRV. See [an example and explanation](convenience-features.html#pointers-limited).

When a memory pointer points to a physical memory location, the pointer will be translated to a PhysicalStorageBuffer storage class in SPIRV.
When a slang module uses a pointer, the resulting SPIRV will be using the SpvAddressingModelPhysicalStorageBuffer64 addressing mode. Modules with pointers but they don't point to a physical memory location will use SpvAddressingModelLogical addressing mode.


Matrix type translation
-----------------------

TODO: Quickly summarize what DXC does as described in [HLSL to SPIR-V Feature Mapping Manual](https://github.com/Microsoft/DirectXShaderCompiler/blob/main/docs/SPIR-V.rst#appendix-a-matrix-representation) and say Slang follows the same behavior.

TODO: Then we can go for the reasoning behind this design, in that what hlsl calls a row is what glsl will call a column. Essentially, a hlsl row or glsl column is:
 - what matrix[i] returns.
 - what matrices can be constructed from float3x4(v1, v2, v3), or mat3x4(v1,v2,v3).

TODO: If we map it the other way, simple operations like matrix[i] or construction will become very messy code that may lead to slower performance.

TODO: The only consequence of mapping float3x4 to matrix(vector4, 3) (or mat3x4, a 3-"column"-by-4-"row" matrix in spirv terminology) is that matrix-vector and matrix-matrix multiplication operations needs to have their operand ordering swapped, which is what slang will do:
```
mul(m, v) ==> v*m
```

TODO: Once we talk about this, then there is the row-major and column-major layout. Note that the decision to use row-major or column-major layout has nothing to do with how the matrix type itself is defined in hlsl or spirv. It really just a data layout modifier just like std140 or std430.

By default, Slang uses row-major layout as opposed to defaulting to column major layout in dxc.
Please see more details in [a1-01-matrix-layout](a1-01-matrix-layout.md).


Legalization
------------

Legalization is a process where Slang applies slightly different approach to translate the input Slang shader to the target.
This process allows Slang shaders to be written in a syntax that SPIR-V may not be able to achieve natively.

Slang allows to use opaque resource types as members of a struct.

Slang allows functions that return any resource types as return type or `out` parameter as long as things are statically resolvable.

Slang allows functions that return arrays.

Slang allows putting scalar/vector/matrix/array types directly as element type of a constant buffer or structured buffers.

When RasterizerOrder resources are used, the order of the rasterization is guaranteed by the instructions from `SPV_EXT_fragment_shader_interlock` extension.

A `StructuredBuffer` with a primitive type such as `StructuredBuffer<int> v` is translated to a buffer with a struct that has the primitive type, which is more like `struct Temp { int v; }; StructuredBuffer<Temp> v;`. It is because, SPIR-V requires buffer variables to be declared within a named buffer block.

When `pervertex` keyword is used, the given type for the varying input will be translated into an array of the given type whose element size is 3. It is because each triangle consists of three vertices.


Tessellation
------------

In HLSL and Slang, Hull shader requires two functions: a Hull shader and patch function.
A typical example of a Hull shader will look like the following.
```
// Hull Shader (HS)
[domain("quad")]
[patchconstantfunc("constants")]
HS_OUT main(InputPatch<VS_OUT, 4> patch, uint i : SV_OutputControlPointID)
{
  ...
}
HSC_OUT constants(InputPatch<VS_OUT, 4> patch)
{
  ...
}
```

When targeting SPIR-V, the patch function is merged as a part of the Hull shader, because GLSL nor SPIR-V differentiates them like how HLSL does.

As an example, a Hull shader will be emitted as following,
```
void main() {
    ...
    main(patch, gl_InvocationID);
    barrier();
    if (gl_InvocationID == 0)
    {
        constants(path);
    }
}
```

This behavior is same to how DXC translates from HLSL to SPIR-V.


Summary
-------

This chapter described any specific details for each target.


