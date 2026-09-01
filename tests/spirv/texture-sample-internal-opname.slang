//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute -emit-spirv-directly

// Regression test for shader-slang/slang#12002.
//
// The texture-sample intrinsics in hlsl.meta.slang define an internal spirv_asm
// result register for the raw OpImageSample* result. The SPIR-V emitter
// auto-emits an OpName for every named spirv_asm register, so that internal
// register name surfaces in the generated module's OpName debug section.
//
// This register used to be named %sampled, which produced `OpName ... "sampled"`
// on the sample result inside the callee. Because "sampled" is a plausible user
// variable name, it looked (to #12002's reporter) like their local
// `float sampled = sample(...)` had migrated into the callee. It had not: the
// name is stdlib-internal. Renaming the register to %__sampled keeps the emitted
// OpName self-evidently compiler-internal (leading `__`), so it can no longer be
// mistaken for a user symbol.

[[vk::binding(0, 0)]] Texture2D<float> srcImage;
[[vk::binding(1, 0)]] SamplerState samp;
[[vk::binding(2, 0)]] RWStructuredBuffer<float> outputBuffer;

[noinline]
float sample(Texture2D<float> image, SamplerState sampler)
{
    return image.SampleLevel(sampler, float2(0, 0), 0.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // The internal sample-result register's OpName is emitted as "__sampled",
    // making it clearly compiler-internal rather than a user variable name.
    // CHECK: OpName %{{[A-Za-z0-9_]+}} "__sampled"
    //
    // And the old plain "sampled" name must not appear: the character before
    // `sampled` in the emitted name is `_` (from `__sampled`), never `"`, so a
    // quote-anchored negative check is exact and does not match "__sampled".
    // CHECK-NOT: "sampled"
    float sampled = sample(srcImage, samp);
    outputBuffer[0] = sampled;
}
