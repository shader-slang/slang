//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T12:00:00+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=52293a26550b22915d50787927526213ba913b7231afe19be04ac5605d79d8b4
//META: purpose=`Atomic<T>::load()` and `::store(v)` lower to the dedicated `atomicLoad(%ptr, %order)` and `atomicStore(%ptr, %val, %order)` opcodes (separate from the RMW ops).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's atomic-operations table lists `atomicLoad` and
// `atomicStore` as the two non-RMW members of the atomic family.
// Both surface from the corresponding `Atomic<T>` methods. The CHECKs
// pin the documented operand counts — `atomicLoad(ptr, memoryOrder)`
// and `atomicStore(ptr, val, memoryOrder)` — by allowing exactly one
// operand after the pointer (load) / value (store).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int v = abuf[idx].load();
    abuf[idx].store(42);
    rwbuf[idx] = v;
}

// IR-LABEL: func %main
// IR: atomicLoad(%{{[^,]+}}, {{[^,)]+}})
// IR: atomicStore(%{{[^,]+}}, 42 : Int, {{[^,)]+}})
