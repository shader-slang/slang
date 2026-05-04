//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl

// RWTexture2D with rgba16f format generates texture_storage_2d<rgba16float, read_write>
// in WGSL, but the WGSL spec only allows read and write (not read_write) for rgba16float.
// Slang should emit an error instead of producing invalid WGSL.
// GitHub issue: https://github.com/shader-slang/slang/issues/7164

[[vk::image_format("rgba16f")]]
RWTexture2D<float4> textureOut;
//CHECK: texture format 'rgba16float' does not support 'read_write' access for storage textures in WGSL

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    textureOut[threadId.xy] = float4(1, 0, 0, 1);
}
