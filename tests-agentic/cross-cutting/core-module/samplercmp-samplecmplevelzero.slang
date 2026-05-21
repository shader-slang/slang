//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `Texture2D<float>.SampleCmpLevelZero(SamplerComparisonState, uv, ref)` lowers per target (HLSL SampleCmpLevelZero, GLSL textureLod over sampler2DShadow with LOD 0, SPIR-V OpImageSampleDrefExplicitLod Lod 0).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SampleCmpLevelZero` is the compute-safe variant: it pins LOD to 0
// so no implicit derivatives are required. SPIR-V emits the
// Dref+ExplicitLod form.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

Texture2D<float> shadow;
SamplerComparisonState scmp;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = shadow.SampleCmpLevelZero(scmp, float2(0.5, 0.5), 0.5);
}

// HLSL: SamplerComparisonState
// HLSL: SampleCmpLevelZero(
// GLSL: sampler2DShadow
// SPIRV: OpImageSampleDrefExplicitLod
