Slang 64-bit Type Support
=========================

The Slang language supports 64 bit built in types. Such as

* double
* uint64_t
* int64_t

This also applies to vector and matrix versions of these types. 

Unfortunately if a specific target supports the type or the typical HLSL instrinsic functions (such as sin/cos/max/min etc) depends very much on the target. 

Double support
==============

Target   | Compiler/Binary  |  Double Type   |   Intrinsics          |  Notes
---------|------------------|----------------|-----------------------|-----------
CPU      |                  |      Yes       |          Yes          |  1
CUDA     | Nvrtx/PTX        |      Yes       |          Yes          |  1
D3D12    | DXC/DXIL         |      Yes       |          No           |  2 
Vulkan   | GlSlang/Spir-V   |      Yes       |          No           |  3
D3D11    | FXC/DXBC         |      No        |          No           |
D3D12    | FXC/DXBC         |      No        |          No           | 

1) CUDA and CPU support most intrinsics, with the notable exception currently of matrix invert
2) Requires SM 6.0 and above  https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12
3) Restriction is described in  https://www.khronos.org/registry/spir-v/specs/1.0/GLSL.std.450.html

D3D12 and VK may have some very limited intrinsic support - sqrt, rsqrt


uint64_t Support
=================

Target   | Compiler/Binary  |  uint64_t Type |  Intrinsic support | Notes
---------|------------------|----------------|--------------------|--------
CPU      |                  |      Yes       |          Yes       |   
CUDA     | Nvrtx/PTX        |      Yes       |          Yes       |   
D3D12    | DXC/DXIL         |      Yes       |          No        |   
Vulkan   | GlSlang/Spir-V   |      Yes       |          No        |   
D3D11    | FXC/DXBC         |      No        |          No        |   1
D3D12    | FXC/DXBC         |      No        |          No        |   1

The intrinsics available on uint64_t type are `abs`, `min`, `max`, `clamp` and `countbits`.

1) uint64_t support requires https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12, so DXBC is not a target.

int64_t Support
================

Target   | Compiler/Binary  |  uint64_t Type |  Intrinsic support | Notes
---------|------------------|----------------|--------------------|--------
CPU      |                  |      Yes       |          Yes       |   
CUDA     | Nvrtx/PTX        |      Yes       |          Yes       |   
Vulkan   | GlSlang/Spir-V   |      Yes       |          No        |   
D3D12    | DXC/DXIL         |      No        |          No        | 1    
D3D11    | FXC/DXBC         |      No        |          No        | 1 
D3D12    | FXC/DXBC         |      No        |          No        | 1

1) SM6.0 only supports uint64_t - and DXBC doesn't support SM6.0, so no support on D3D targets.


The intrinsics available on uint64_t type are `abs`, `min`, `max` and `clamp`.



