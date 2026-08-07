//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=857d9a7b5620991cb161cbee5d5f843c16e973d3c494110ba22777ebfea3d38f
//META: purpose=The lower-tuple-types pass decomposes Slang tuples into scalar values; no `Tuple` type appears in the HLSL emit.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Lower tuple types" row of the type-and-value legalization
// table (`slang-ir-lower-tuple-types.cpp`) decomposes Slang
// `Tuple<...>` types. The HLSL emit contains no `Tuple` type name.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Tuple<int, int> t = makeTuple(a, a + 1);
    buf[0] = t._0 + t._1;
}

// HLSL: void main
// HLSL-NOT: Tuple
