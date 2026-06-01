//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=f394bf1461f5876676bdb5fa5c51feaa920cf5ef0986df9fe1bbf0db976d5a30
//META: purpose=`Atomic<T>::min(v)` and `::max(v)` lower to `atomicMin(%ptr, %val, %order)` and `atomicMax(%ptr, %val, %order)` respectively.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `atomicMin` and `atomicMax` as siblings of
// `atomicAdd` in the atomic-RMW family. Both surface from
// `Atomic<int>::min(v)` and `::max(v)`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int pmin = abuf[idx].min(10);
    int pmax = abuf[idx].max(100);
    rwbuf[idx] = pmin + pmax;
}

// IR-LABEL: func %main
// IR: atomicMin(%{{[^,]+}}, 10 : Int, {{.*}})
// IR: atomicMax(%{{[^,]+}}, 100 : Int, {{.*}})
