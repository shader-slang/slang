Slang Target Compatibility 
==========================

Items with a + means that the feature is aniticipated to be added in the future.
Items with ^ means there is some discussion about support later in the document for this target.

| Feature                     |    D3D11     |    D3D12     |     VK     |      CUDA     |    CPU
|-----------------------------|--------------|--------------|------------|---------------|---------------
| Half Type                   |     No       |     Yes      |   Yes      |     No+       |    No+
| Double Type                 |     Yes      |     Yes      |   Yes      |     Yes       |    Yes
| Double Intrinsics           |     No       |   Limited+   |  Limited   |     Most      |    Yes
| u/int64_t Type              |     No       |  Yes (SM6.0) |   Yes      |     Yes       |    Yes
| u/int64_t Intrinsics        |     No       |   No         |   Yes      |     Yes       |    Yes
| int matrix                  |     Yes      |   Yes        |   No+      |     Yes       |    Yes
| tex.GetDimension            |     Yes      |   Yes        |   Yes      |     No        |    Yes
| SM6.0 Wave Intrinsics       |     No       |   Yes        |  Partial   |     Yes       |    No
| SM6.0 Quad Intrinsics       |     No       |   Yes        |   No+      |     No        |    No
| SM6.5 Wave Intrinsics       |     No       |   Yes (1)    |   No+      |     Yes       |    No
| Tesselation                 |     Partial+ |   Partial+   | Limited+   |     No        |    No
| Graphics Pipeline           |     Yes      |   Yes        |   Yes      |     No        |    No
| Ray Tracing                 |     No       |   Yes        |   Yes      |     No        |    No
| Native Bindless             |     No       |    No        |   No       |     Yes       |    Yes
| Buffer bounds               |     Yes      |   Yes        |   Yes      |   Limited     |    Limited
| Resource bounds             |     Yes      |   Yes        |   Yes      | Yes (optional)|    Yes
| Atomics                     |     Yes      |   Yes        |   Yes      |     Yes       |    Yes
| Group shared mem/Barriers   |     Yes      |   Yes        |   Yes      |     Yes       |    No+ 
| TextureArray.Sample float   |     Yes      |   Yes        |   Yes      |     No        |    Yes
| Separate Sampler            |     Yes      |   Yes        |   Yes?     |     No        |    Yes
| tex.Load                    |     Yes      |   Yes        |   Yes      |  Limited      |    Yes
| Full bool                   |     Yes      |   Yes        |   Yes      |     No        |    Yes^ 

## Half Type

There appears to be a problem writing to a StructuredBuffer containing half on D3D12. D3D12 also appears to have problems doing calculations with half.

## tex.GetDimensions

tex.GetDimensions is the GetDimensions method on 'texture' objects. This is not supported on CUDA as CUDA has no equivalent functionality to get these values. GetDimensions work on Buffer resource types on CUDA.

## u/int64_t Type

Requires SM6.0 which requires DXIL for D3D12. Therefore not available with DXBC on D3D11 or D3D12.

## int matrix

Means can use matrix types containing integer types. 

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

## tex.Load

tex.Load is only supported on CUDA for Texture1D. Additionally CUDA only allows such access for linear memory, meaning the bound texture can also not have mip maps. Load is allowed on RWTexture types on CUDA.

## Full bool

Means fully featured bool support. CUDA has issues around bool because there isn't a vector bool type built in. Currently bool aliases to an int vector type. 

On CPU there are some issues in so far as bool's size is not well defined in size an alignment. Most C++ compliers now use a byte to represent a bool. In the past it has been backed by an int on some compilers. 
