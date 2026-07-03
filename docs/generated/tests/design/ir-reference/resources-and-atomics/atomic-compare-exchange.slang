//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomiccompareexchange
//META: doc_section_digest=f64e810eeb8bfcd2dc78872345ca3a0ea2009ac1e4a545333ad63bf1f54ce88c
//META: purpose=`Atomic<T>::compareExchange(expected, desired)` lowers to `atomicCompareExchange(%ptr, %expected, %desired, ...)`, the CAS primitive returning the previous value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `atomicCompareExchange` notable-opcode note describes
// the CAS primitive: pointer + expected + desired, returning the
// value that was previously stored.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].compareExchange(1, 2);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicCompareExchange(%{{[^,]+}}, 1 : Int, 2 : Int, {{.*}})
