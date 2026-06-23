//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=15ffe04c81dead304d9773538e8bca993b707b3393574380bcd38d0a23da4044
//META: purpose=Expansion: DCE removes an entirely-unread module-scope uniform (`leftover`) before emit — the name does not appear in the HLSL `GlobalParams` struct.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Cleanup axis on DCE. A second uniform `leftover` is declared but
// never read. DCE removes it before the global-uniform packing pass
// emits a `GlobalParams` struct, so its name does not appear in the
// HLSL emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int used;
uniform int leftover;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = used + int(tid.x);
}

// HLSL: void main
// HLSL-NOT: leftover
