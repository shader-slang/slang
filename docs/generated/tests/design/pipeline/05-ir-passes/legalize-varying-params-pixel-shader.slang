//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=87cc303db4e3bb3825cd2dfa57f040c4ffad9180d465ffc6172dea9a41fbd518
//META: purpose=Expansion: legalize-varying-params on a pixel-shader entry point routes user-defined TEXCOORD inputs into per-target shapes and `SV_Target` into the per-target output marker on HLSL and GLSL.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pixel-shader variation on legalize-varying-params: a user-defined
// `TEXCOORD0` interpolated input and an `SV_Target` color output.
// Both surface as per-target stage-input / stage-output markers
// after the pass runs.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage pixel -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage pixel -entry main

float4 main(float2 uv : TEXCOORD0) : SV_Target
{
    return float4(uv.x, uv.y, 0.0f, 1.0f);
}

// HLSL-DAG: TEXCOORD0
// HLSL-DAG: SV_TARGET
// GLSL: void main
