//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
//META: purpose=Expansion: lower-tuple-types decomposes a 3-element `Tuple<int, float, uint>` so no `Tuple` type spelling survives on HLSL — the heterogeneous element types are flattened.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Three-element tuple, heterogeneous elements. The lower-tuple-types
// pass should decompose into flat values; the spelling `Tuple` does
// not appear in the HLSL emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int ai;
uniform float af;
uniform uint au;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    let t = makeTuple(ai, af, au);
    buf[0] = float(t._0) + t._1 + float(t._2);
}

// HLSL: void main
// HLSL-NOT: Tuple<
