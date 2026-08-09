//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#the-atomic-memory-order-operand
//META: doc_section_digest=644c7061c24b763fe96ab44c8cef95120acec98260f723b4edecc5a432356e4e
//META: purpose=An explicit `MemoryOrder` argument reaches the atomic opcode as the raw `IRMemoryOrder` integer — `Acquire` is `1 : Int`, `Release` is `2 : Int`, `AcquireRelease` is `3 : Int`, and `SeqCst` is `4 : Int`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies every non-relaxed value of the `IRMemoryOrder` enum that the
// atomics section reproduces. Each of the four orders is passed to a
// different atomic method, and each CHECK pins the enumerator's integer
// in the trailing operand position; together they walk the enum from its
// first non-zero value to its maximum, which is the boundary the enum
// listing commits to.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int a = abuf[idx].load(MemoryOrder.Acquire);
    abuf[idx].store(a + idx, MemoryOrder.Release);
    int b = abuf[idx].exchange(a, MemoryOrder.AcquireRelease);
    int c = abuf[idx].add(b, MemoryOrder.SeqCst);
    rwbuf[idx] = c;
}

// IR-LABEL: func %main
// IR: atomicLoad(%{{[^,]+}}, 1 : Int)
// IR: atomicStore(%{{[^,]+}}, %{{[^,]+}}, 2 : Int)
// IR: atomicExchange(%{{[^,]+}}, %{{[^,]+}}, 3 : Int)
// IR: atomicAdd(%{{[^,]+}}, %{{[^,]+}}, 4 : Int)
