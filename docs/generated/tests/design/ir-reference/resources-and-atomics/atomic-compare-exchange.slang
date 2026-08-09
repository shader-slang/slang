//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomiccompareexchange
//META: doc_section_digest=7df7fb40b43dccb446b1e09fcfd971cee6e54f70b1a4e2cc56f1755c4feb75a3
//META: purpose=`Atomic<T>::compareExchange(expected, desired)` lowers to `atomicCompareExchange(%ptr, %expected, %desired, ...)` — the CAS primitive returning the previous value, with the documented trailing memory-order operand(s) left unpinned.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the CAS primitive lowers to `atomicCompareExchange` with
// the documented `ptr, expected, desired` triple up front, returning
// the previously stored value. The doc's operand column now adds two
// trailing memory-order operands (matched / unmatched case); the CHECK
// leaves the trailing operand count unpinned because the declared Lua
// operand list carries only the triple — see the bundle README's
// doc-gap row on the memory-order operand encoding.

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
