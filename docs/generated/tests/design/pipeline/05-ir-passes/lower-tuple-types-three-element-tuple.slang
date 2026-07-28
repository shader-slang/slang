//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=87cc303db4e3bb3825cd2dfa57f040c4ffad9180d465ffc6172dea9a41fbd518
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
