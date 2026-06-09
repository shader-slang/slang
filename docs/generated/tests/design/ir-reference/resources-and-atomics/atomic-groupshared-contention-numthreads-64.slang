//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=64 contending threads atomic-adding to a single groupshared `Atomic<int>` still emit a single documented `atomicAdd` opcode in IR.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress probe: workgroup-local contention pattern; one atomic add
// per thread on the same `groupshared` location.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

groupshared Atomic<int> gsa;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int prev = gsa.add(1);
    rwbuf[tid.x] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, 1 : Int, {{.*}})
