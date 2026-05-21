//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T09:38:00Z
//META: source_commit=1106750632bd5fb062ea9e50319f7763d34f78d5
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=120aee6df2c1b3d3ea42ea63294c00830c98fd3053f546fc49ad4100f4318f68
//META: purpose=Expansion: HLSL meta-module `Texture2DArray.SampleGrad(sampler, float3{uv, slice}, ddx, ddy)` lowers per target (HLSL SampleGrad, GLSL textureGrad over sampler2DArray, SPIR-V OpImageSampleExplicitLod Grad).
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 2D-array SampleGrad takes float2 derivatives (no derivative on the
// slice axis).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage pixel

Texture2DArray<float4> tex;
SamplerState samp;

[shader("pixel")]
float4 main(float3 uvSlice : TEXCOORD) : SV_Target
{
    float2 ddx = float2(0.01, 0.0);
    float2 ddy = float2(0.0, 0.01);
    return tex.SampleGrad(samp, uvSlice, ddx, ddy);
}

// HLSL: SampleGrad(
// GLSL: textureGrad(sampler2DArray
// SPIRV: OpImageSampleExplicitLod {{.*}} Grad
