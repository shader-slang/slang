//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
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
