//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T12:00:00+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=52293a26550b22915d50787927526213ba913b7231afe19be04ac5605d79d8b4
//META: purpose=`Atomic<T>::min(v)` and `::max(v)` lower to `atomicMin(%ptr, %val, %order)` and `atomicMax(%ptr, %val, %order)` respectively.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `atomicMin` and `atomicMax` as siblings of
// `atomicAdd` in the atomic-RMW family. Both surface from
// `Atomic<int>::min(v)` and `::max(v)`; each CHECK pins a single
// trailing memory-order operand per the documented
// `ptr, val, memoryOrder` shape.

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
// IR: atomicMin(%{{[^,]+}}, 10 : Int, {{[^,)]+}})
// IR: atomicMax(%{{[^,]+}}, 100 : Int, {{[^,)]+}})
