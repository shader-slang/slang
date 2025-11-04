//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Test that unbounded arrays of various opaque types do not emit ArrayStride decoration
// Opaque types (textures, samplers, acceleration structures, etc.) are handles/descriptors
// and should not have ArrayStride decorations in SPIR-V.

// CHECK: OpCapability RuntimeDescriptorArray
// CHECK-NOT: OpDecorate %{{.*}}Texture{{.*}} ArrayStride
// CHECK-NOT: OpDecorate %{{.*}}Sampler{{.*}} ArrayStride

[[vk::binding(0, 0)]] Texture2D<float4> textures[];
[[vk::binding(1, 0)]] SamplerState samplers[];
[[vk::binding(2, 0)]] RWTexture2D<float4> rwTextures[];

[numthreads(1, 1, 1)]
[shader("compute")]
void computeMain(uint3 id : SV_DispatchThreadID) {
    float4 color = textures[0].Sample(samplers[0], float2(0.5, 0.5));
    rwTextures[0][id.xy] = color;
}

