//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `Texture3D` plus `Sample(sampler, uvw)` lowers per target (HLSL keeps Sample, GLSL emits texture(sampler3D), SPIR-V emits OpImageSampleImplicitLod over a 3D OpTypeImage).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture3D<T>` is one of the HLSL-named resource types layered in by
// the HLSL meta-module. Its `Sample(sampler, uvw)` overload takes a
// `float3` and uses implicit derivatives, so the entry point must be
// pixel-stage. Each backend has a distinct lowering.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage pixel

Texture3D<float4> tex;
SamplerState samp;

[shader("pixel")]
float4 main(float3 uvw : TEXCOORD) : SV_Target
{
    return tex.Sample(samp, uvw);
}

// HLSL: Texture3D
// HLSL: Sample(
// GLSL: sampler3D
// SPIRV: OpTypeImage {{.*}} 3D
// SPIRV: OpImageSampleImplicitLod
