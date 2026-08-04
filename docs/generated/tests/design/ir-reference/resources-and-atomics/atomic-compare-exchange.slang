//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T12:00:00+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomiccompareexchange
//META: doc_section_digest=71b74ee056270d4345b7257154c1737e99ecafed1d54c59932e1d630864a86cf
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
