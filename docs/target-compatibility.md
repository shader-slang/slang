Slang Target Compatibility 
==========================


Shader Model (SM) numbers are D3D Shader Model versions, unless explicitly stated otherwise.
OpenGL compatibility is not listed here, because OpenGL isn't an officially supported target. 

Items with a + means that the feature is anticipated to be added in the future.
Items with ^ means there is some discussion about support later in the document for this target.

| Feature                     |    D3D11     |    D3D12     |     VK     |      CUDA     |    CPU
|-----------------------------|--------------|--------------|------------|---------------|---------------
| Half Type                   |     No       |     Yes      |   Yes      |     No +      |    No +
| Double Type                 |     Yes      |     Yes      |   Yes      |     Yes       |    Yes
| Double Intrinsics           |     No       |   Limited +  |  Limited   |     Most      |    Yes
| u/int64_t Type              |     No       |   Yes ^      |   Yes      |     Yes       |    Yes
| u/int64_t Intrinsics        |     No       |   No         |   Yes      |     Yes       |    Yes
| int matrix                  |     Yes      |   Yes        |   No +     |     Yes       |    Yes
| tex.GetDimension            |     Yes      |   Yes        |   Yes      |     No        |    Yes
| SM6.0 Wave Intrinsics       |     No       |   Yes        |  Partial   |     Yes ^     |    No
| SM6.0 Quad Intrinsics       |     No       |   Yes        |   No +     |     No        |    No
| SM6.5 Wave Intrinsics       |     No       |   Yes ^      |   No +     |     Yes ^     |    No
| WaveMask Intrinsics         |     Yes ^    |   Yes ^      |   Yes +    |     Yes       |    No
| WaveShuffle                 |     No       |   Limited ^  |   Yes      |     Yes       |    No
| Tesselation                 |     Yes ^    |   Yes ^      |   No +     |     No        |    No
| Graphics Pipeline           |     Yes      |   Yes        |   Yes      |     No        |    No
| Ray Tracing DXR 1.0         |     No       |   Yes ^      |   Yes ^    |     No        |    No
| Ray Tracing DXR 1.1         |     No       |   Yes        |   No +     |     No        |    No
| Native Bindless             |     No       |    No        |   No       |     Yes       |    Yes
| Buffer bounds               |     Yes      |   Yes        |   Yes      |   Limited ^   |    Limited ^
| Resource bounds             |     Yes      |   Yes        |   Yes      | Yes (optional)|    Yes
| Atomics                     |     Yes      |   Yes        |   Yes      |     Yes       |    Yes
| Group shared mem/Barriers   |     Yes      |   Yes        |   Yes      |     Yes       |    No + 
| TextureArray.Sample float   |     Yes      |   Yes        |   Yes      |     No        |    Yes
| Separate Sampler            |     Yes      |   Yes        |   Yes      |     No        |    Yes
| tex.Load                    |     Yes      |   Yes        |   Yes      |  Limited ^    |    Yes
| Full bool                   |     Yes      |   Yes        |   Yes      |     No        |    Yes ^ 
| Mesh Shader                 |     No       |   No +       |   No +     |     No        |    No
| `[unroll]`                  |     Yes      |   Yes        |   Yes ^    |     Yes       |    Limited + 
| Atomics                     |     Yes      |   Yes        |   Yes      |     Yes       |    No + 
| Atomics on RWBuffer         |     Yes      |   Yes        |   Yes      |     No        |    No + 

## Half Type

There appears to be a problem writing to a StructuredBuffer containing half on D3D12. D3D12 also appears to have problems doing calculations with half.

## u/int64_t Type

Requires SM6.0 which requires DXIL for D3D12. Therefore not available with DXBC on D3D11 or D3D12.

## int matrix

Means can use matrix types containing integer types. 

## tex.GetDimensions

tex.GetDimensions is the GetDimensions method on 'texture' objects. This is not supported on CUDA as CUDA has no equivalent functionality to get these values. GetDimensions work on Buffer resource types on CUDA.

## SM6.0 Wave Intrinsics

CUDA has premliminary support for Wave Intrinsics, introduced in [PR #1352](https://github.com/shader-slang/slang/pull/1352). Slang synthesizes the 'WaveMask' based on program flow and the implied 'programmer view' of exectution. This support is built on top of WaveMask intrinsics with Wave Intrinsics being replaced with WaveMask Intrinsic calls with Slang generating the code to calculate the appropriate WaveMasks.

Please read [PR #1352](https://github.com/shader-slang/slang/pull/1352) for a better description of the status.

## SM6.5 Wave Intrinsics

SM6.5 Wave Intrinsics are supported, but requires a downstream DXC compiler that supports SM6.5. As it stands the DXC shipping with windows does not. 

## WaveMask Intrinsics

In order to map better to the CUDA sync/mask model Slang supports 'WaveMask' intrinsics. They operate in broadly the same way as the Wave intrinsics, but require the programmer to specify the lanes that are involved. To write code that uses wave intrinsics acrosss targets including CUDA, currently the WaveMask intrinsics must be used. For this to work, the masks passed to the WaveMask functions should exactly match the 'Active lanes' concept that HLSL uses, otherwise the result is undefined. 

The WaveMask intrinsics are not part of HLSL and are only available on Slang.

## WaveShuffle

`WaveShuffle` and `WaveBroadcastLaneAt` are Slang specific intrinsic additions to expand the options available around `WaveReadLaneAt`. 

To be clear this means they will not compile directly on 'standard' HLSL compilers such as `dxc`, but Slang HLSL *output* (which will not contain these intrinsics) can (and typically is) compiled via dxc.

The difference between them can be summarized as follows

* WaveBroadcastLaneAt - laneId must be a compile time constant 
* WaveReadLaneAt - laneId can be dynamic but *MUST* be the same value across the Wave ie 'dynamically uniform' across the Wave
* WaveShuffle - laneId can be truly dynamic (NOTE! That it is not strictly truly available currently on all targets, specifically HLSL)

Other than the different restrictions on laneId they act identically to WaveReadLaneAt.

`WaveBroadcastLaneAt` and `WaveReadLaneAt` will work on all targets that support wave intrinsics, with the only current restriction being that on GLSL targets, only scalars and vectors are supported.

`WaveShuffle` will always work on CUDA/Vulkan. 

On HLSL based targets currently `WaveShuffle` will be converted into `WaveReadLaneAt`. Strictly speaking this means it *requires* the `laneId` to be `dynamically uniform` across the Wave. In practice some hardware supports the loosened usage, and others does not. In the future this may be fixed in Slang and/or HLSL to work across all hardware. For now if you use `WaveShuffle` on HLSL based targets it will be necessary to confirm that `WaveReadLaneAt` has the loosened behavior for all the hardware intended. If target hardware does not support the loosened restrictions it's behavior is undefined. 

## Tesselation

Although tesselation stages should work on D3D11 and D3D12 they are not tested within our test framework, and may have problems. 

## Native Bindless  

Bindless is possible on targets that support it - but is not the default behavior for those targets, and typically require significant effort in Slang code. 

'Native Bindless' targets use a form of 'bindless' for all targets. On CUDA this requires the target to use 'texture object' style binding and for the device to have 'compute capability 3.0' or higher.

## Resource bounds 

For CUDA this is optional as can be controlled via the SLANG_CUDA_BOUNDARY_MODE macro in the `slang-cuda-prelude.h`. By default it's behavior is `cudaBoundaryModeZero`.

## Buffer Bounds

This is the feature when accessing outside of the bounds of a Buffer there is well defined behavior - on read returning all 0s, and on write, the write being ignored.

On CPU there is only bounds checking on debug compilation of C++ code. This will assert if the access is out of range.

On CUDA out of bounds accesses default to element 0 (!). The behavior can be controlled via the SLANG_CUDA_BOUND_CHECK macro in the `slang-cuda-prelude.h`. This behavior may seem a little strange - and it requires a buffer that has at least one member to not do something nasty. It is really a 'least worst' answer to a difficult problem and is better than out of range accesses or worse writes.

## TextureArray.Sample float 

When using 'Sample' on a TextureArray, CUDA treats the array index parameter as an int, even though it is passed as a float.

## Separate Sampler

This feature means that a multiple Samplers can be used with a Texture. In terms of the HLSL code this can be seen as the 'SamplerState' being a parameter passed to the 'Sample' method on a texture object. 

On CUDA the SamplerState is ignored, because on this target a 'texture object' is the Texture and Sampler combination.

## Graphics Pipeline

CPU and CUDA only currently support compute shaders. 

## Ray Tracing DXR 1.0

Vulkan does not support a local root signature, but there is the concept of a 'shader record'. In Slang a single constant buffer can be marked as a shader record with the `[[vk::shader_record]]` attribute, for example:

```
[[vk::shader_record]]
cbuffer ShaderRecord
{
	uint shaderRecordID;
} 
```

In practice to write shader code that works across D3D12 and VK you should have a single constant buffer marked as 'shader record' for VK and then on D3D that constant buffer should be bound in the local root signature on D3D. 

## tex.Load

tex.Load is only supported on CUDA for Texture1D. Additionally CUDA only allows such access for linear memory, meaning the bound texture can also not have mip maps. Load *is* allowed on RWTexture types of other dimensions including 1D on CUDA.

## Full bool

Means fully featured bool support. CUDA has issues around bool because there isn't a vector bool type built in. Currently bool aliases to an int vector type. 

On CPU there are some issues in so far as bool's size is not well defined in size an alignment. Most C++ compilers now use a byte to represent a bool. In the past it has been backed by an int on some compilers. 

## `[unroll]`

The unroll attribute allows for unrolling `for` loops. At the moment the feature is dependent on downstream compiler support which is mixed. In the longer term the intention is for Slang to contain it's own loop unroller - and therefore not be dependent on the feature on downstream compilers. 

On C++ this attribute becomes SLANG_UNROLL which is defined in the prelude. This can be predefined if there is a suitable mechanism, if there isn't a definition SLANG_UNROLL will be an empty definition. 

On GLSL and VK targets loop unrolling uses the [GL_EXT_control_flow_attributes](https://github.com/KhronosGroup/GLSL/blob/master/extensions/ext/GL_EXT_control_flow_attributes.txt) extension.

Slang does have a cross target mechanism to [unroll loops](language-reference/06-statements.md), in the section `Compile-Time For Statement`.

## Atomics on RWBuffer

For VK the GLSL output from Slang seems plausible, but VK binding fails in tests harness.

On CUDA RWBuffer becomes CUsurfObject, which is a 'texture' type and does not support atomics. 

On the CPU atomics are not supported, but will be in the future.
