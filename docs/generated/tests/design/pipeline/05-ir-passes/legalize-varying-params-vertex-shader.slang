//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
//META: purpose=Expansion: legalize-varying-params routes a vertex-shader output `SV_Position` into the per-target stage-output marker on HLSL and GLSL — alternative pipeline-stage shape.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Legalize varying params" row of the type-and-value
// legalization table (`slang-ir-legalize-varying-params.cpp`)
// handles stage inputs/outputs. A vertex-shader entry point with
// `SV_Position` output exercises the output direction.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage vertex -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage vertex -entry main

float4 main(uint vid : SV_VertexID) : SV_Position
{
    return float4(float(vid), 0.0f, 0.0f, 1.0f);
}

// HLSL: SV_POSITION
// GLSL: gl_Position
