//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=534618adad82acb65da181567998c07a77381ecf27996abdf7535de5b49ec7b3
//META: purpose=`Atomic<T>::increment()` and `::decrement()` lower to the nullary-value opcodes `atomicInc(%ptr, %order)` and `atomicDec(%ptr, %order)`, which carry no value operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two atomics whose only operands are the destination
// pointer and the memory order. Both methods are called on the same
// buffer element and their results are summed into an output buffer so
// nothing is dead; the CHECKs assert exactly two operands per opcode,
// which is what distinguishes `atomicInc`/`atomicDec` from the
// read-modify-write atomics that take a value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int a = abuf[idx].increment();
    int b = abuf[idx].decrement();
    rwbuf[idx] = a + b;
}

// IR-LABEL: func %main
// IR: atomicInc(%{{[^,]+}}, {{[^,)]+}})
// IR: atomicDec(%{{[^,]+}}, {{[^,)]+}})
