//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
//META: purpose=Expansion: lower-buffer-element-type renders an `RWStructuredBuffer<float>` element as `float _data[]` on GLSL — alternative element-type axis to the int case.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Element-type variation on lower-buffer-element-type. A
// `RWStructuredBuffer<float>` produces a `float _data[]` SSBO field
// on GLSL.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform float a;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + float(tid.x);
}

// GLSL: float _data
