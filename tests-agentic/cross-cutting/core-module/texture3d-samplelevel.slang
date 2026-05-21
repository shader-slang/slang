//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `Texture3D.SampleLevel(sampler, uvw, lod)` lowers per target (HLSL SampleLevel, GLSL textureLod over sampler3D, SPIR-V OpImageSampleExplicitLod with Lod operand).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture3D.SampleLevel` provides an explicit LOD, making it safe in
// non-pixel stages (no implicit derivatives required). The compute
// stage stresses the LOD-explicit lowering path.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

Texture3D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(0.5, 0.5, 0.5), 0.0);
}

// HLSL: SampleLevel(
// GLSL: textureLod(sampler3D
// SPIRV: OpTypeImage {{.*}} 3D
// SPIRV: OpImageSampleExplicitLod {{.*}} Lod
