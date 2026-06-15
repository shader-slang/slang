//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=Stress: legalize-vector-types renders all three of `vector<int, 2>`, `vector<int, 3>`, `vector<int, 4>` simultaneously by pinning them into typed RW buffers — the three native spellings (`int2`, `int3`, `int4`) all appear in the HLSL emit.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Multiple vector lengths in the same module exercise the vector-
// types legalizer's many-shape path. We escape each vector through a
// typed `RWStructuredBuffer` to prevent DCE from collapsing them
// before the CHECK can observe the emitted type names.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<vector<int, 2> > buf2;
uniform RWStructuredBuffer<vector<int, 3> > buf3;
uniform RWStructuredBuffer<vector<int, 4> > buf4;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf2[0] = vector<int, 2>(int(tid.x), int(tid.y));
    buf3[0] = vector<int, 3>(int(tid.x), int(tid.y), int(tid.z));
    buf4[0] = vector<int, 4>(int(tid.x), int(tid.y), int(tid.z), 7);
}

// HLSL-DAG: int2
// HLSL-DAG: int3
// HLSL-DAG: int4
