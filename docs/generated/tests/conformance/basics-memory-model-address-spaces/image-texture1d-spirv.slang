//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:38+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-address-spaces.md#address-spaces
//META: doc_section_digest=3851dfb34bba532778d07931eed37a5ffb7720f62c8acf95efe83c116ec49a46
//META: purpose=`Texture1D<T>` also belongs to the Image address space (not just Texture2D); SPIRV emits it with `UniformConstant` storage class.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `Texture1D<T>` (a different dimensionality than the primary Texture2D test)
// also falls in the Image address space. CHECK asserts the SPIRV declares `%tex1d` as
// `OpVariable ... UniformConstant`, confirming the Image→UniformConstant mapping holds
// across texture dimensionalities, not only for 2-D textures.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

Texture1D<float>   tex1d;
SamplerState       samp;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID, RWStructuredBuffer<float> output)
{
    output[0] = tex1d.SampleLevel(samp, 0.5, 0.0);
}

// CHECK: %tex1d = OpVariable {{.*}} UniformConstant
