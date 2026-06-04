//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `Texture1D.Sample(sampler, u)` lowers per target (HLSL Sample, GLSL texture over sampler1D, SPIR-V OpImageSampleImplicitLod over a 1D OpTypeImage).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `Texture1D<T>` takes a scalar `float` UV. Pixel stage gets implicit
// derivatives; the SPIR-V emit must mark the image as `1D`.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage pixel

Texture1D<float4> tex;
SamplerState samp;

[shader("pixel")]
float4 main(float u : TEXCOORD) : SV_Target
{
    return tex.Sample(samp, u);
}

// HLSL: Texture1D
// HLSL: Sample(
// GLSL: sampler1D
// SPIRV: OpTypeImage {{.*}} 1D
// SPIRV: OpImageSampleImplicitLod
