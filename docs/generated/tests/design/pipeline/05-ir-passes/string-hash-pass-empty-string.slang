//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
//META: purpose=Expansion (boundary): the string-hash pass produces a numeric constant for `getStringHash("")` (empty input) — the empty string literal does not survive in the HLSL emit.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis on string-hash: the empty string. The pass still
// replaces the intrinsic with a numeric hash value; no
// `getStringHash` call remains and the empty literal `""` does not
// leak.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = getStringHash("");
}

// HLSL: void main
// HLSL-NOT: getStringHash
