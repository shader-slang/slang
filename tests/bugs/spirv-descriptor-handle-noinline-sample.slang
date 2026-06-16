// Regression test for https://github.com/shader-slang/slang/issues/11496
//
// Sampling a `DescriptorHandle<Texture2D>` inside a `[noinline]` callee under
// `spvDescriptorHeapEXT` previously SIGSEGV'd in the SPIR-V emitter (a
// null-typed orphan `IRParam` reached the asm-operand image-type rebuild).
// The crash is now resolved upstream; this test asserts the repro compiles to
// valid SPIR-V — the texture type and the sample both emit.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -profile spirv_1_6 -capability spvDescriptorHeapEXT

SamplerState sampler;

static const uint N = 2;

[noinline]
float4 bad_function(Texture2D tex, float4 values[N])
{
    return tex.Sample(sampler, float2(0.0));
}

struct Data
{
    float4 values[N];
};

struct PushConstants
{
    DescriptorHandle<Texture2D> tex;
    ImmutablePtr<Data> data;
};

[shader("fragment")]
float4 main(uniform PushConstants pc)
    : SV_Target
{
    return bad_function(pc.tex, pc.data->values);
}

//CHECK: OpTypeImage
//CHECK: OpImageSampleImplicitLod
