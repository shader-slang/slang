//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=534618adad82acb65da181567998c07a77381ecf27996abdf7535de5b49ec7b3
//META: purpose=`Atomic<T>::min(v)` and `::max(v)` lower to `atomicMin(%ptr, %val, %order)` and `atomicMax(%ptr, %val, %order)` respectively.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `min` and `max` are separate read-modify-write opcodes
// rather than a compare-and-store sequence. Both methods are called on
// the same buffer element and each CHECK allows exactly one operand
// after the value, which pins the `(ptr, val, order)` shape shared by
// the whole read-modify-write family.

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
