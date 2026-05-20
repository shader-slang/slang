//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=`Atomic<T>::load()` and `::store(v)` lower to the dedicated `atomicLoad(%ptr, %order)` and `atomicStore(%ptr, %val, %order)` opcodes (separate from the RMW ops).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's atomic-operations table lists `atomicLoad` and
// `atomicStore` as the two non-RMW members of the atomic family.
// Both surface from the corresponding `Atomic<T>` methods.

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
// IR: atomicLoad(%{{[^,]+}}, {{.*}})
// IR: atomicStore(%{{[^,]+}}, 42 : Int, {{.*}})
