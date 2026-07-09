//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
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
