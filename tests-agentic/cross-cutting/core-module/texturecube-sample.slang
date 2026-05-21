//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `TextureCube.Sample(sampler, dir)` lowers per target (HLSL Sample, GLSL texture over samplerCube, SPIR-V OpImageSampleImplicitLod over a Cube OpTypeImage).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `TextureCube<T>` takes a `float3` direction vector and dispatches to
// the cube-mapped lookup; the lowering carries a `Cube` dimensionality
// on every backend.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage pixel

TextureCube<float4> tex;
SamplerState samp;

[shader("pixel")]
float4 main(float3 dir : TEXCOORD) : SV_Target
{
    return tex.Sample(samp, dir);
}

// HLSL: TextureCube
// HLSL: Sample(
// GLSL: samplerCube
// SPIRV: OpTypeImage {{.*}} Cube
// SPIRV: OpImageSampleImplicitLod
