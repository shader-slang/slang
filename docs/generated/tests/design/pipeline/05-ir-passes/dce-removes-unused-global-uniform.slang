//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#ssa-construction-and-basic-cleanup
//META: doc_section_digest=1784b8e9f30332a4995be817424580937cb7178f3cb7ce01ea908f8b8807e9fa
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
