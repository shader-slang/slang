SP #005: Write-Only Textures
=================

Add Write-Only texture types to Slang's core module.


Status
------

Status: Design Review.

Implemtation: N/A

Author: Yong He

Reviewer: 

Background
----------

Slang inherits HLSL's RWTexture types to represent UAV/storage texture resources, this works well for HLSL, GLSL, CUDA and SPIRV targets.
However Metal has the notion of write only textures, and WebGPU has limited support of read-write textures. In WebGPU, a read-write texture can only have
uncompressed floating point format, which means a `RWTexture2D` cannot be used to efficiently write to a `rgba8unorm` texture.

To provide better mapping to write-only textures on Metal and WebGPU, we propose to add write only textures to Slang to allow writing portable code
without relying on backend workarounds.

Proposed Approach
-----------------

Slang's core module already defines all texture types as a single generic `_Texture<T, ..., access, ...>` type, where `access` is a value parameter
representing the allowed access of the texture. The valid values of access are:

```
kCoreModule_ResourceAccessReadOnly
kCoreModule_ResourceAccessReadWrite
kCoreModule_ResourceAccessRasterizerOrdered
kCoreModule_ResourceAccessFeedback
```

We propose to add another case:

```
kCoreModule_ResourceAccessWriteOnly
```

to represent write-only textures.


Also add the typealiases prefixed with "W" for all write only textures:
```
WTexture1D, WTexture2D, ...
```

These types will be reported in the reflection API with `access=SLANG_RESOURCE_ACCESS_WRITE`.

Write only textures are supported on all targets. For traditional HLSL, GLSL, SPIRV and CUDA targets, they are translated
exactly the same as `RW` textures. For Metal, they map to `access::write`, and for WGSL, they map to `texture_storage_X<format, write>`.