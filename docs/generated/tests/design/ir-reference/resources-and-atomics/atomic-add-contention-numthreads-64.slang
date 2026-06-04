//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=64 contending threads each calling `Atomic<int>::add(1)` on the same element still emit a single documented `atomicAdd` opcode in IR (the contention is a runtime property, not an IR-shape property).
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: [numthreads(64,1,1)] with every thread adding 1 to
// `abuf[0]` is the canonical contention pattern. At the IR level
// only one `atomicAdd` opcode is emitted; the per-thread fan-out is
// implicit in the dispatch. The documented opcode shape is
// unchanged.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int prev = abuf[0].add(1);
    rwbuf[tid.x] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, 1 : Int, {{.*}})
