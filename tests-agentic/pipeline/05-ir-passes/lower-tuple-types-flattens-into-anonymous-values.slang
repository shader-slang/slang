//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
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
