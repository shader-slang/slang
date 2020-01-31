Slang 64-bit Type Support
=========================

The Slang language supports 64 bit built in types. Such as

* double
* uint64_t
* int64_t

This also applies to vector and matrix versions of these types. 

Unfortunately if a specific target supports the type or the typical HLSL instrinsic functions (such as sin/cos/max/min etc) depends very much on the target. 

Note this initial testing only tested scalar usage, and not vector or matrix intrinsics.

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
Note that GlSlang does produce spir-v that contains double intrinsic calls, the failure happens when validating the Spir-V 

```
Validation: error 0:  [ UNASSIGNED-CoreValidation-Shader-InconsistentSpirv ] Object: VK_NULL_HANDLE (Type = 0) | SPIR-V module not valid: GLSL.std.450 Sin: expected Result Type to be a 16 or 32-bit scalar or vector float type
  %57 = OpExtInst %double %1 Sin %56
```

D3D12 and VK may have some very limited intrinsic support such as sqrt, rsqrt

uint64_t Support
=================

Target   | Compiler/Binary  |  uint64_t Type |  Intrinsic support | Notes
---------|------------------|----------------|--------------------|--------
CPU      |                  |      Yes       |          Yes       |   
CUDA     | Nvrtx/PTX        |      Yes       |          Yes       |   
D3D12    | DXC/DXIL         |      Yes       |          Yes       |   
Vulkan   | GlSlang/Spir-V   |      Yes       |          Yes       |   
D3D11    | FXC/DXBC         |      No        |          No        |   1
D3D12    | FXC/DXBC         |      No        |          No        |   1

1) uint64_t support requires https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12, so DXBC is not a target.

The intrinsics available on uint64_t type are `abs`, `min`, `max`, `clamp` and `countbits`.

int64_t Support
================

Target   | Compiler/Binary  |  int64_t Type |  Intrinsic support | Notes
---------|------------------|----------------|--------------------|--------
CPU      |                  |      Yes       |          Yes       |   
CUDA     | Nvrtx/PTX        |      Yes       |          Yes       |   
Vulkan   | GlSlang/Spir-V   |      Yes       |          Yes       |   
D3D12    | DXC/DXIL         |      Yes       |          Yes       | 1
D3D11    | FXC/DXBC         |      No        |          No        | 2 
D3D12    | FXC/DXBC         |      No        |          No        | 2

1) The sm6.0 docs (https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12) describe only supports uint64_t, but the dxc compiler page says int64_t is supported in HLSL 2016 (https://github.com/Microsoft/DirectXShaderCompiler/wiki/Language-Versions). Tests show that this is indeed the case.

2) uint64_t support requires https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/hlsl-shader-model-6-0-features-for-direct3d-12, so DXBC is not a target.

The intrinsics available on uint64_t type are `abs`, `min`, `max` and `clamp`.

GLSL
====

GLSL/Spir-v based targets do not support 'generated' intrinsics on matrix types. For example 'sin(mat)' will not work on GLSL/Spir-v.

