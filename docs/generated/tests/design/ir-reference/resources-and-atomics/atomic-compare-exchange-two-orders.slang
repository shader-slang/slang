//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomiccompareexchange
//META: doc_section_digest=7df7fb40b43dccb446b1e09fcfd971cee6e54f70b1a4e2cc56f1755c4feb75a3
//META: purpose=`atomicCompareExchange` is the only atomic with two memory orders: `ptr, expected, desired` are followed by a success order at operand 3 and a failure order at operand 4.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the five-operand CAS shape. `expected` and `desired` are both
// derived from `SV_DispatchThreadID` so neither collapses to a literal,
// which leaves the two trailing operands as the only constants on the
// line; the CHECK pins them as `0 : Int, 0 : Int`, showing that both the
// success and the failure order are materialized separately rather than
// shared.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].compareExchange(idx, idx + 1);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicCompareExchange(%{{[^,]+}}, %{{[^,]+}}, %{{[^,]+}}, 0 : Int, 0 : Int)
