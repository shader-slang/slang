//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/ir-reference/resources-and-atomics.md#atomiccompareexchange
//META: doc_section_digest=f64e810eeb8bfcd2dc78872345ca3a0ea2009ac1e4a545333ad63bf1f54ce88c
//META: purpose=`Atomic<int>::compareExchange(7, 7)` (matching expected==desired — degenerate CAS) still lowers to the documented `atomicCompareExchange` opcode shape.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: when `expected` equals `desired`, the CAS has no
// observable side effect on the value but the opcode must still be
// emitted (no folding).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].compareExchange(7, 7);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicCompareExchange(%{{[^,]+}}, 7 : Int, 7 : Int, {{.*}})
