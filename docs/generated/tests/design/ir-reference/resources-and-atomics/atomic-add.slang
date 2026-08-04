//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T12:00:00+00:00
//META: source_commit=5f0e613fdd3c2890bb087d9ff76154e2605a14d2
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=52293a26550b22915d50787927526213ba913b7231afe19be04ac5605d79d8b4
//META: purpose=`Atomic<T>::add(v)` lowers to `atomicAdd(%ptr, %val, %order)`, the IR encoding of the atomic add primitive.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `Atomic<int>::add(v)` lowers to the documented three-operand
// `atomicAdd(%ptr, %val, %memoryOrder)` shape. The portable surface is
// the `Atomic<T>::add(v)` method on a `RWStructuredBuffer` element; the
// CHECK pins exactly one trailing operand after the value — the
// `IRMemoryOrder` constant (`0 : Int` is relaxed) that the operand
// column now names explicitly.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<Atomic<int>> abuf;
uniform RWStructuredBuffer<int> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    int prev = abuf[idx].add(idx + 1);
    rwbuf[idx] = prev;
}

// IR-LABEL: func %main
// IR: atomicAdd(%{{[^,]+}}, %{{[^,]+}}, {{[^,)]+}})
