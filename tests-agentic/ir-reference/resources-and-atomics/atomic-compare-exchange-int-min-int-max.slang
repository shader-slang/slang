//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#atomiccompareexchange
//META: doc_section_digest=f64e810eeb8bfcd2dc78872345ca3a0ea2009ac1e4a545333ad63bf1f54ce88c
//META: purpose=`Atomic<int>::compareExchange(INT_MIN, INT_MAX)` (extreme operand pair) still lowers to the documented `atomicCompareExchange` opcode shape; both literals carry through.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: largest and smallest signed values as the CAS
// operand pair.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].compareExchange(-2147483648, 2147483647);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicCompareExchange(%{{[^,]+}}, {{.*}}-2147483648{{.*}}, 2147483647 : Int, {{.*}})
