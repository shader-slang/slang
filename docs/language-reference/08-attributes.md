Attributes
==========

> Note: This section is not yet complete.

## [[vk::spirv_instruction]]

** SPIR-V only **

This attribute is only available for Vulkan SPIR-V output via GLSLANG. In the future it could be supported via the `direct-spirv` option. The attribute will be ignored for any other target. This attribute requires the `GL_EXT_spirv_intrinsics` GLSL extension.

The attibute allows access to SPIR-V intrinsics, by supplying a function declaration with the appropriate signature for the SPIR-V op and no body. The intrinsic takes a single parameter which is the integer value for the SPIR-V op. 

In the example below the add function, uses the mechanism to directly use the SPIR-V integer add 'op' which is 128 in this case.

```HLSL
// 128 is OpIAdd in SPIR-V
[[vk::spirv_instruction(128)]]
uint add(uint a, uint b);

RWStructuredBuffer<uint> resultBuffer;

[numthreads(4,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint threadId = dispatchThreadID.x;
    resultBuffer[threadId] = add(threadId, threadId);
}
```

