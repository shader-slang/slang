//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl

// RWTexture2D with half4 element type should emit a semantic-level error when
// targeting WGSL, since WGSL only supports f32/i32/u32 element types for textures.
// GitHub issue: https://github.com/shader-slang/slang/issues/7164

[[vk::image_format("rgba16f")]]
RWTexture2D<half4> textureOut;
//CHECK: WGSL supports only f32/i32/u32 type textures

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    textureOut[threadId.xy] = half4(1, 0, 0, 1);
}
