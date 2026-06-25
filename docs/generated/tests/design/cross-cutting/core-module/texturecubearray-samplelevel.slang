//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `TextureCubeArray.SampleLevel(sampler, float4{dir, slice}, lod)` lowers per target (HLSL SampleLevel, GLSL textureLod over samplerCubeArray, SPIR-V OpImageSampleExplicitLod Lod).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Compute-stage SampleLevel exercises the explicit-LOD path for an
// arrayed cube image.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

TextureCubeArray<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float4(1.0, 0.0, 0.0, 0.0), 0.0);
}

// HLSL: SampleLevel(
// GLSL: textureLod(samplerCubeArray
// SPIRV: OpImageSampleExplicitLod {{.*}} Lod
