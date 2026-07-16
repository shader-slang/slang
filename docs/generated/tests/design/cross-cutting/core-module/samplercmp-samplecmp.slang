//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Expansion: HLSL meta-module `Texture2D<float>.SampleCmp(SamplerComparisonState, uv, ref)` lowers per target (HLSL SampleCmp, GLSL textureLod over sampler2DShadow, SPIR-V OpImageSampleDrefImplicitLod).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SamplerComparisonState` plus `SampleCmp` is the depth-comparison
// path. Pixel-stage is required because `SampleCmp` uses implicit
// derivatives. SPIR-V emits an `OpImageSampleDref*` form.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage pixel

Texture2D<float> shadow;
SamplerComparisonState scmp;

[shader("pixel")]
float main(float2 uv : TEXCOORD, float ref : TEXCOORD1) : SV_Target
{
    return shadow.SampleCmp(scmp, uv, ref);
}

// HLSL: SamplerComparisonState
// HLSL: SampleCmp(
// GLSL: sampler2DShadow
// SPIRV: OpImageSampleDref{{[A-Za-z]+}}
