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
HLSL 6.2 introduced [16-bit scalar types](https://github.com/microsoft/DirectXShaderCompiler/wiki/16-Bit-Scalar-Types) such as float16 and int16_t, but they didn't come with any atomic operations.
HLSL 6.6 introduced [atomic operations for 64-bit integer types and bitwise atomic operations for 32-bit float type](https://microsoft.github.io/DirectX-Specs/d3d/HLSL_SM_6_6_Int64_and_Float_Atomics.html), but 16-bit integer types and 16-bit float types are not a part of it.

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


ParameterBlock for SPIR-V target
--------------------------------

`ParameterBlock` is a Slang generic type for binding uniform parameters.
It is similar to `ConstantBuffer` in HLSL, but `ParameterBlock` can include not only constant parameters but also descriptors such as Texture2D or StructuredBuffer.

Because the Vulkan API doesn't natively support `ParameterBlock` that has a mixture of descriptors and constants, Slang emits each member as an individual parameter.
When targeting SPIR-V, the individual parameter will be emitted as either `uniform` or `buffer`.


SPIR-V specific Compiler options
--------------------------------

The following compiler options are specific to SPIR-V.

### -emit-spirv-directly
Generate SPIR-V output directly (default)
It cannot be used with -emit-spirv-via-glsl

### -emit-spirv-via-glsl
Generate SPIR-V output by compiling generated GLSL with glslang
It cannot be used with -emit-spirv-directly

### -g
Include debug information in the generated code, where possible.
When targeting SPIR-V, this option emits [SPIR-V NonSemantic Shader DebugInfo Instructions](https://github.com/KhronosGroup/SPIRV-Registry/blob/main/nonsemantic/NonSemantic.Shader.DebugInfo.100.asciidoc).

### -O<optimization-level>
Set the optimization level.
When targeting SPIR-V, this option applies to the downstream compiler.
"-O0" option can help to identify issues when the problem is from the optimization step of the downstream compiler.
Compared to the macro inlining, the specialization helps debugging with "-O0" option, because the specialization preserves the code structure much more than the macro inlining.

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
These options will be applied globally.
If you need to apply a different buffer layout for indivisual StructuredBuffer, you can specify the layout as a second generic argument to StructuredBuffer. E.g., StructuredBuffer<T, Std140Layout>

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

### [vk::aliased_pointer]
When two memory pointers point to a same memory location, they are considered `aliased`. Compiler needs to be informed of such cases in order to generate the code that produces a consistant outcome. Slang can figure out whether a pointer needs to be treated as aliased or not, but you can explicitly mark a variable as aliased.

### [vk::restrict_pointer]
Opposite to [vk::aliased_pointer] in a way that it tells the compiler to treat the variable as not-aliased.

### [vk::spirv_instruction(op : int, set : String = "")]
When applied to a function, the function will use the `op` value corresponding to a SPIR-V instruction. You can also specify which instruction set you want to use as a second argument. E.G., `[[vk::spirv_instruction(1, "NonSemantic.DebugBreak")]]`


Multiple entry points support
-----------------------------

Slang supports mutiple entry points when targeting SPIR-V.
It is same to HLSL and SPIR-V that allows a shader source to have multiple entry points.
Note that GLSL requires the entry point to be named, "main", and a shader source can have only one entry point.


Memory pointer is experimental
------------------------------

SPIR-V currently supports five types of `Addressing model`-s, and it can be grouped as Logical addressing model and non-local addressing model.

The logical addressing model means that the pointers are abstract, and they have no physical size nor numeric value.

The non-Logical addressing models allow physical pointers to be formed. When a pointer is declared, it is declared internally with a specific `Storage Class`.  SPIR-V has a limitation that "Pointers for one Storage Class must not be used to access objects in another Storage Class."

Slang supports the Logical and non-Logical addressing models with the limitations from SPIR-V and addional limitations. See more [explanation and an example](convenience-features.html#pointers-limited).

Some examples of the `Storage class` are listed below:
- UniformConstant: is for uniform constant variables as read-only.
- Uniform: is for uniform buffer as read-only.
- StorageBuffer: is for UAV/structuredBuffer/ShaderStorageBuffer as read-write.
- PhysicalStorageBuffer: same as StorageBuffer but it follows non-logical addressing model.
- PushConstant: is for variables as read-only stored in the buffer that has `push_constant` layout.

Note that only ones starting with `Physical-` follows the non-logical addressing model.


Difference between `std140` layout and `std430` layout
------------------------------------------------------

When constant parameters are send to the shader, there are different rules applied to the alignment and paddings.
`std140` requires alignment of 16 bytes on `struct` types and it adds padding to round up to 16 bytes for each member.
`std430` requires a natural alignment, which means that the required alignment is same as its size.


Matrix type translation
-----------------------

When targeting SPIR-V, Slang assumes Row-Major matrix, which is same behavior as HLSL/DXC.
You can change the behavior to Column-major with a compiler option, "-matrix-layout-column-major".

One of frequently asked questions is that "GLSL/OpenGL uses Column-major and why Slang uses Row-major for targeting SPIR-V?"
GLSL specification says that "Initialization of matrix values is done with constructors in column-major order", and also says "(In Matrix,) two subscripts select a column and then a row."
Although it is considered "Column-major", when we access the matrix as 2-dimensional array, the first subscript selects "column" and not "row".
This is effectively same to a Row-major system when the first subscript selects row and the second subscript selects column.

The actual difference is on the fact that the "memory layout" is in Column-major when OpenGL sends the matrix data to shader.
When the shader reads the data from the memory, it needs to know how to handle.
That's why you can specify `row_major` and `column_major` for uniform parameters as their `layout` qualifier in GLSL.
By default, OpenGL sends the matrix data in Column-major layout, but it is up-to the application to decide how to layout the matrix data.

For more detailed explanation, please checkout another document [a1-01-matrix-layout](a1-01-matrix-layout.md)


Legalization (Need to use a more user friendly word)
------------

TODO: We should also have sections to talk about type legalization and mention all the cases that we enable with our legalization passes.

TODO: For example, we allow using resource types in structs, we allow functions that return resource types as return type or out parameter, as long as things are statically resolvable. We allow functions that return arrays, we allow putting scalar/vector/matrix/array types directly as element type of a constant buffer or structured buffers. Make sure to go over our legalization passes to build a list of allowed things that people are unsure whether it is allowed.


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
    // Set tessellation levels
    if (gl_InvocationID == 0) {
        gl_TessLevelOuter[0] = ...;
        gl_TessLevelOuter[1] = ...;
        gl_TessLevelOuter[2] = ...;
        gl_TessLevelOuter[3] = ...;

        gl_TessLevelInner[0] = ...;
        gl_TessLevelInner[1] = ...;
    }

    out_POSITION[gl_InvocationID] = in_POSITION[gl_InvocationID];
}
```

This behavior is same to how DXC translates from HLSL to SPIR-V.


Summary
-------

This chapter described any specific details for each target.


