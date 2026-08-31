//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
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
