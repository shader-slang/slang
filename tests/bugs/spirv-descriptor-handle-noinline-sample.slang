// Regression test for https://github.com/shader-slang/slang/issues/11496
//
// `slangc` SIGSEGV'd in the SPIR-V emitter when sampling a `DescriptorHandle<Texture2D>`
// inside a [noinline] callee under `spvDescriptorHeapEXT`: an upstream IR pass left
// an orphan `IRParam` (no `getFullType()`, no `getParent()`) wired into the
// `kIROp_SPIRVAsmOperandImageType / SampledImageType` arm of `emitSpvAsmOperand`,
// and the `as<IRTextureTypeBase>(...)` cast result was dereferenced unguarded.
//
// This PR is a partial fix: the defensive null-guard at the reader converts the
// SIGSEGV into a clean `SLANG_UNEXPECTED` diagnostic that references this issue
// and a follow-up tracking the upstream orphan-param producer. The reporter's
// shader still does not compile successfully — that is gated on the upstream fix.
// This test asserts the crash-to-diagnostic conversion only.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -profile spirv_1_6 -capability spvDescriptorHeapEXT

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

//CHECK: slang#11496
