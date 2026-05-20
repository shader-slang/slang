//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: HLSL meta-module Texture2D.Sample accepts the center UV (0.5,0.5) and lowers per target.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: texture UV — center coordinate (0.5, 0.5). Use the
// pixel stage so GLSL accepts the implicit-derivative Sample call.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage pixel
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage pixel

Texture2D<float4> tex;
SamplerState samp;

[shader("pixel")]
float4 main(float2 uv : TEXCOORD) : SV_Target
{
    float2 centerUV = float2(0.5, 0.5);
    return tex.Sample(samp, centerUV);
}

// HLSL: Sample(
// GLSL: texture(
