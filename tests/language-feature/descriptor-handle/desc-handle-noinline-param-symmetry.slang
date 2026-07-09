//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain -emit-spirv-directly

// Regression for shader-slang/slang#12004.
//
// A bindless `Texture2D<float>` and `SamplerState` (both obtained via `.Handle` from a
// cbuffer of `DescriptorHandle`s) passed into a `[noinline]` function must be passed
// SYMMETRICALLY: both as bindless indices that the callee re-materializes into a resource
// in its own body. Before the fix, only the texture was passed by index while the scalar
// sampler was loaded at the call site and passed by value as an `OpTypeSampler` — so the
// callee signature was `float(uint, OpTypeSampler)` instead of `float(uint, uint)`.

[[vk::binding(0, 1)]]
cbuffer Parameters
{
    SamplerState.Handle u_sampler;
    Texture2D<float>.Handle u_srcImage;
    WTexture2D<float>.Handle u_dstImage;
}

[noinline]
float sample(Texture2D<float> image, SamplerState sampler)
{
    return image.SampleLevel(sampler, float2(0.0, 0.0), 0.0);
}

// The `sample` callee takes two bindless indices — both `%uint` — not a `uint` index plus a
// by-value `OpTypeSampler`. Capture the `sample` function-type id from its OpTypeFunction (a
// float returning two uints), then bind the `sample` OpFunction to exactly that type so the
// assertion is anchored to this specific callee, and require both of its OpFunctionParameters
// to be `%uint`. This is the exact structural signature the issue reports as asymmetric.
//CHECK: [[FNTYPE:%[a-zA-Z0-9_]+]] = OpTypeFunction %float %uint %uint
//CHECK: %sample = OpFunction %float {{.*}} [[FNTYPE]]
//CHECK-NEXT: OpFunctionParameter %uint
//CHECK-NEXT: OpFunctionParameter %uint

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float sampled = sample(u_srcImage, u_sampler);
    u_dstImage.Store(uint2(0, 0), sampled);
}
