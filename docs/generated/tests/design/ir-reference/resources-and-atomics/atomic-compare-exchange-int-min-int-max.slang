//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomiccompareexchange
//META: doc_section_digest=7df7fb40b43dccb446b1e09fcfd971cee6e54f70b1a4e2cc56f1755c4feb75a3
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
