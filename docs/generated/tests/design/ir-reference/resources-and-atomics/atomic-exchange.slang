//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=534618adad82acb65da181567998c07a77381ecf27996abdf7535de5b49ec7b3
//META: purpose=`Atomic<T>::exchange(v)` lowers to `atomicExchange(%ptr, %val, %order)`, the swap primitive that returns the previously stored value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the unconditional swap. The new value is thread-derived so
// it stays an SSA operand, and the CHECK allows exactly one operand
// after it — the memory order — which is what separates `atomicExchange`
// from the two-order `atomicCompareExchange`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].exchange(idx + 7);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicExchange(%{{[^,]+}}, %{{[^,]+}}, {{[^,)]+}})
