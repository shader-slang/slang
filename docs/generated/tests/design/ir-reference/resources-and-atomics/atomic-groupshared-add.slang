//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T12:00:00+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=52293a26550b22915d50787927526213ba913b7231afe19be04ac5605d79d8b4
//META: purpose=`Atomic<T>` on a `groupshared` array element also lowers to `atomicAdd`, demonstrating that the atomic opcode is independent of the underlying address-space backing the `Atomic<T>` element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The atomic-operations doc states the pointer operand is "a
// pointer to the target location"; the address-space behind that
// pointer can be either a buffer element or a `groupshared`
// variable. The opcode is the same, including the single trailing
// memory-order operand of the documented `ptr, val, memoryOrder`
// shape.

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
// IR: atomicAdd(%{{[^,]+}}, %{{[^,]+}}, {{[^,)]+}})
