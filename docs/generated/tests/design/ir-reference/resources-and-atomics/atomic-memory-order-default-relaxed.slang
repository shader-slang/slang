//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#the-atomic-memory-order-operand
//META: doc_section_digest=644c7061c24b763fe96ab44c8cef95120acec98260f723b4edecc5a432356e4e
//META: purpose=The undeclared trailing memory-order operand is always materialized in lowered IR and defaults to `0 : Int` (`kIRMemoryOrder_Relaxed`), at operand 1 for `atomicLoad` and operand 2 for `atomicStore` and the read-modify-write atomics.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the positional layout of the memory-order operand when the
// caller omits it. Because the core-module default is materialized at
// the call site during checking, the operand is present in the dump
// even though no `MemoryOrder` appears in the source. The CHECKs pin
// `0 : Int` as the last operand and pin its index by counting the
// operands ahead of it: one for `atomicLoad`, two for `atomicStore`
// and `atomicAdd`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int a = abuf[idx].load();
    abuf[idx].store(a + idx);
    int b = abuf[idx].add(idx + 1);
    rwbuf[idx] = a + b;
}

// IR-LABEL: func %main
// IR: atomicLoad(%{{[^,]+}}, 0 : Int)
// IR: atomicStore(%{{[^,]+}}, %{{[^,]+}}, 0 : Int)
// IR: atomicAdd(%{{[^,]+}}, %{{[^,]+}}, 0 : Int)
