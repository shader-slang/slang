//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=`Atomic<T>` on a `groupshared` array element also lowers to `atomicAdd`, demonstrating that the atomic opcode is independent of the underlying address-space backing the `Atomic<T>` element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The atomic-operations doc states the pointer operand is "a
// pointer to the target location"; the address-space behind that
// pointer can be either a buffer element or a `groupshared`
// variable. The opcode is the same.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

groupshared Atomic<int> gsa;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    int prev = gsa.add((int)gid.x + 1);
    rwbuf[tid.x] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, %{{[^,]+}}, {{.*}})
